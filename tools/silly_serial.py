import logging
import argparse
import time
import serial
import pathlib

import re


LOG = None
def setup_log( log_name ):
    global LOG
    if LOG == None:
        logger = logging.getLogger('supa')
        logger.setLevel(logging.DEBUG)
        formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')

        ch = logging.StreamHandler()
        ch.setLevel(logging.INFO)
        ch.setFormatter(formatter)
        
        fn = logging.FileHandler("/tmp/supa_serial.log", mode='w')
        fn.setLevel(logging.DEBUG)
        fn.setFormatter(formatter)
        
        logger.addHandler(ch)
        logger.addHandler(fn)
        LOG = logger
    return logging.getLogger( "supa." + log_name )
log = setup_log("serial")


def get_config():
   parser = argparse.ArgumentParser("interactive shell")
   parser.add_argument("--stopbits", type=int, default=1 )
   parser.add_argument("--parity", type=str, default="E")
   parser.add_argument("--baudrate", type=int, default=115200 )
   parser.add_argument("--timeout", type=float, default=0.1 )
   parser.add_argument("--port", default=None, help="prefix to replace on the source")
   parser.add_argument("action")
   parser.add_argument("--output", default="./data/pid_p{kp:03d}_i{ki:03d}_d{kd:03d}.json.gz")
   
   return parser.parse_args()


class ShellSerial:

    PROMPT="uart:~$"
    
    def __init__( self ):
      self.fid = None
      self.lines = []
      self.buffer = b""
      
    def _read( self ):
        #log.info("Reading data..")
        self.buffer += self.fid.read( 512 )
        
        # Then split
        lines = self.buffer.split(b"\n")
        if len(lines[-1]) != 0:
            self.buffer = lines[-1]
            lines = lines[0:-1]
            
        if len(lines) == 0:
            return None
        
        lines = [ x.strip() for x in lines ]
        lines = [ ( time.time(), x.decode("utf-8")) for x in lines if len(x) > 0 ]
        
        self.lines += lines
        log.debug("Received '%s'", lines )
        return True
    
    
    def wait( self, to_wait ):
        
        while True:
            
            ret = self.wait_find( to_wait )
            if ret:
                return ret
            
            self._read()
            
            
    def wait_find( self, to_wait):
        
        # log.info("Current lines '%s'", self.lines )
        
        to_loop = len(self.lines)
        found_index = -1
        
        for loop in range(to_loop):
            
            if to_wait in self.lines[loop][1]:
                found_index = loop
                break
        else:
            #log.error("Could not find line '%s'!", to_wait)
            return None
        
        before = self.lines[0 : found_index+1]
        self.lines=self.lines[(found_index+1):]
        return before
    
    
    
    def command( self, command, wait = PROMPT ):
        log.info("Writing command '%s'", command )
        command = command + "\n\n"
        self.fid.write( command.encode("utf-8")  )
        
        if wait == None:
            time.sleep(0.1)
            self._read()
            self.lines = []
            to_ret = None
        else:
            to_ret = self.wait( wait )
            log.info("Command done")
            return to_ret
        
    def reset( self ):
        self.command( "reset", wait=None )
        
    def open_connection( self, config ):
        if config.port == None:
            for port_path in [ "/dev/ttyUSB0","/dev/rfcomm0"]:
               if pathlib.Path( port_path ).exists():
                   config.port = port_path
                   break
            else:
                raise Exception("No valid port found.")
                
        self.fid = serial.Serial( port=config.port, baudrate=config.baudrate, timeout=config.timeout, parity=config.parity, stopbits=config.stopbits )
        log.info("Connection '%s' opened ok!", config.port)



def parse_pid_output( output ):
    #PID 2.8   10.0 9.4    0.6 -3.1 1.3
    pattern = re.compile(".*PID\s*([\d\.]+)\s+([\d\.]+)\s+([\d\.]+)\s+([\d\.]+)\s+([\d\.]+)\s+([\d\.]+)\s*")
    
    values = []

    for ts,ln in output:
        mat = pattern.match( ln )
        if mat:
           val = [ts,] + [ float( mat.group(x) ) for x in range(1,7) ]
           values.append(val)
    
    if len(values) == 0:
        raise Exception("Values is empty!")
    
    return values

import gzip
import json

def save_file( output_fn , output ):
    log.info("Saving output to '%s'", output_fn )
    with open( output_fn, "wb") as fid:
        fid.write( gzip.compress( json.dumps( output ).encode("utf-8") ) )    
    
def main( config ):
    shell = ShellSerial()
    shell.open_connection( config )
    shell.reset()
    
    if config.action == "pid":
        
        ki = 0
        kd = 0
        
        for kp in [20,40,60,80,100]:
            shell.reset()
            shell.command("motor pid %d %d %d" % (kp*100, kd*100, ki*100) )
            output = shell.command("motor drive 300 300 40")
            output += shell.wait("Final position")
            
            output = parse_pid_output(output)
            
            output_fn_format = { "kp":kp, "ki":ki, "kd" : kd }
            output_fn = config.output.format( **output_fn_format )
            save_file( output_fn, output )
            
        
#    print("All done, output: %s", output )

        
    
if __name__ == "__main__":
    main( get_config() )



