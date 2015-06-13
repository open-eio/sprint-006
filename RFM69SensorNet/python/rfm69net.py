import serial, glob, time

DEFAULT_BAUDRATE = 9600
DEFAULT_PORT_PATTERN = "/dev/ttyUSB*" 

class SerialCommandInterface(object):
    def __init__(self, serial_connection):
        self._ser = serial_connection
    def _send(self, cmd):
        self._ser.write("%s\n" % cmd.strip())
    def _readline(self, strip_endline = True):
        line = self._ser.readline()
        if strip_endline:
            line = line.rstrip()
        return line
    def _exchange(self, cmd,delay=0.01):
        self._send(cmd)
        time.sleep(delay)
        resp = self._readline()
        return resp

class Gateway(SerialCommandInterface):
    def __init__(self, serial_connection):
        super(Gateway, self).__init__(serial_connection)
        
class Node(SerialCommandInterface):
    def __init__(self, serial_connection):
        super(Node, self).__init__(serial_connection)
        

def get_interface(port_pattern = DEFAULT_PORT_PATTERN,
                  baudrate = DEFAULT_BAUDRATE,
                  ):
    ports = glob.glob(DEFAULT_PORT_PATTERN)
    ports.sort()
    ser = None
    #attempt to find a device on the first available port matching the pattern
    for port in ports:
        try:
            print "Attempting to establish connection with port: %s" % port
            ser = serial.Serial(port, baudrate=baudrate)
            #connection establish, get the IDN
            ser.flush()
            ser.write("IDN?\n")
            resp = ser.readline().strip()
            if resp.startswith("RFM69 Gateway"):
                return Gateway(ser)
            elif resp.startswith("RFM69 Node"):
                return Node(ser)
            else:
                raise IOError("Could not identify device with response: '%s'")
        except, err:
            print "Failed with exception: %s" % err
    if ser is None:
        raise IOError("No matching serial device could be found using pattern: %s" % port_pattern)
        
################################################################################
# Test Code
################################################################################
if __name__ == "__main__":
    dev = get_interface("/dev/ttyUSB*")
