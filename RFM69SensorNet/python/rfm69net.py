#standard library dependencies
import sys, glob, time, json
from collections import OrderedDict
#third party dependencies
import serial

DEFAULT_BAUDRATE = 115200
DEFAULT_PORT_PATTERN = "/dev/ttyUSB*" 
DEFAULT_COMMENT_PREFIX = "#" #to ignore debugging printouts

class SerialCommandInterface(object):
    def __init__(self, serial_connection):
        self._ser = serial_connection
    def _send(self, cmd):
        self._ser.write("%s\n" % cmd.strip())
    def _readline(self, strip_endline = True, comment_prefix = DEFAULT_COMMENT_PREFIX):
        line = self._ser.readline()
        while line.startswith(comment_prefix):
            sys.stderr.write(line)
            line = self._ser.readline()
        if strip_endline:
            line = line.rstrip()
        return line
    def _exchange(self, cmd,delay=0.01):
        self._send(cmd)
        time.sleep(delay)
        resp = self._readline()
        return resp
    def _read_json(self):
        buff = []
        char = self._ser.read(1)
        while char != "{": #search for open brace
            sys.stderr.write(char)
            char = self._ser.read(1)
        buff.append(char)
        while char != "}": #search for open brace
            char = self._ser.read(1)
            buff.append(char)
        buff = "".join(buff)
        od = json.loads(buff,object_pairs_hook=OrderedDict)
        return od
    def identify(self):
        return self._exchange("IDN?")

class Gateway(SerialCommandInterface):
    def __init__(self, serial_connection):
        super(Gateway, self).__init__(serial_connection)
        self.dev_type = "gateway"
        
class Node(SerialCommandInterface):
    RADIO_CONFIG_FIELDS = OrderedDict([
      ['nodeID', int],
      ['networkID', int],
      ['gatewayID', int],
      ['frequency', int],
      ['is_RFM69HW', bool],
      ['encryptkey', str],
    ])
    def __init__(self, serial_connection):
        super(Node, self).__init__(serial_connection)
        self.dev_type = "node"
        self.radio_config = None
        self.get_radio_config()
    def get_radio_config(self):
        self._send("RFM69.CONFIG?")
        cfg = self._read_json()
        self.radio_config = cfg
        return cfg
    def set_radio_config(self,**kwargs):
        new_cfg = self.radio_config.copy()
        valid_keys = Node.RADIO_CONFIG_FIELDS.keys()
        for key,val in kwargs.items():
            if key in valid_keys:
                conv = Node.RADIO_CONFIG_FIELDS[key]
                new_cfg[key] = conv(val)
            else:
                raise ValueError("'%s' is not a valid key in %r" % (key,valid_keys))
        #self._send("RFM69.CONFIG?")
        #cfg = self._read_json()


def get_interface(port_pattern = DEFAULT_PORT_PATTERN,
                  baudrate = DEFAULT_BAUDRATE,
                  ):
    ports = glob.glob(DEFAULT_PORT_PATTERN)
    ports.sort()
    ser = None
    global scmd
    #attempt to find a device on the first available port matching the pattern
    for port in ports:
        try:
            print "Attempting to establish connection with port: %s" % port
            ser = serial.Serial(port, baudrate=baudrate)
            #connection establish, get the IDN
            nchars = ser.inWaiting()
            while nchars:
                line = ser.read(nchars)
                sys.stderr.write(line)
                time.sleep(0.1)
                nchars = ser.inWaiting()
            ser.flushInput()
            time.sleep(1.0)
            scmd = SerialCommandInterface(ser)
            resp = scmd.identify()
            while resp == "":
                resp = scmd._exchange("IDN?")
            print resp
            if resp.startswith("RFM69 Gateway"):
                return Gateway(ser)
            elif resp.startswith("RFM69 Node"):
                return Node(ser)
            else:
                raise IOError("Could not identify device with response: '%s'" % resp)
        except Exception, err:
            print "Failed with exception: %s" % err
    if ser is None:
        raise IOError("No matching serial device could be found using pattern: %s" % port_pattern)
        
################################################################################
# Test Code
################################################################################
if __name__ == "__main__":
    dev = get_interface("/dev/ttyUSB*")
