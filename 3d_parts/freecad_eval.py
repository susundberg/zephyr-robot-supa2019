import socket
import os
import argparse
import sys


FREECAD_PORT = 5000
FREECAD_IP = "127.0.0.1"

def init_supalib( path ):
    reloads =  """
import sys
sys.path.append( "%s" )
import supalib
importlib.reload( supalib )
supalib.init()
""" % (path)
    return reloads.encode("Utf-8")

def finish_supalib():
    return "supalib.finish()".encode("utf-8")

def client( message,  prefix = None, postfix=None  ):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((FREECAD_IP, FREECAD_PORT))
    
    try:
        if prefix:
           sock.sendall( prefix )
        sock.sendall( message )
        if postfix:
            sock.sendall( postfix )
        response_raw = sock.recv(1024).decode("utf-8")
        response = "\n".join( ["Received: '%s'" % x for x in response_raw.split("\n")] )
        if response_raw == "None":
            print("All good, bye!")
            return 0
        print("Something went wrong:")
        print(response)
        return 1
        
        
    finally:
        print("Closing socket")
        sock.close()



def main(config):
   with open( config.input_file, 'rb' ) as fid:
       return client( fid.read(), prefix=init_supalib( config.path), postfix=finish_supalib()   )
       
    
    
def cmd_config():
   parser = argparse.ArgumentParser(description='Execute code in remote FreeCad')
   parser.add_argument( "input_file", help="Set the job description file")
   cmd =  parser.parse_args()
   cmd.path = os.path.dirname(os.path.realpath(__file__))
   return cmd
   
if __name__=="__main__":
    sys.exit( main( cmd_config() ) )
