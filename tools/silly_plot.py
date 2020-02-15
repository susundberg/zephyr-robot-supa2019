
import numpy
import pylab
import json
import gzip
import argparse

def get_config():
   parser = argparse.ArgumentParser("interactive shell")
   parser.add_argument("inputs", nargs="+")
   return parser.parse_args()


def load_file( fn ):
    with open( fn, "rb") as fid:
        d = json.loads( gzip.decompress( fid.read() ) )
        print("Loaded %d rows" % len(d))
        d = numpy.array(d)
        d[:,0] -= d[0,0]
        return d
    
def main( config ):
    for fn in config.inputs:
        data = load_file( fn )
        
        pylab.figure()
        pylab.subplot(3,1,1)
        pylab.plot( data[:,0], data[:,1] )
        
        pylab.subplot(3,1,2)
        pylab.plot( data[:,0], data[:,2], label="Target" )
        pylab.plot( data[:,0], data[:,3], label="Measured" )
        pylab.legend()
        
        pylab.subplot(3,1,3)
        pylab.plot( data[:,0], data[:,4], label="Error" )
        pylab.plot( data[:,0], data[:,5], label="Error D" )
        pylab.plot( data[:,0], data[:,6], label="Error I" )
        
        pylab.legend()
        pylab.suptitle( fn )
        
    pylab.show()
    
    
if __name__=="__main__":
    main( get_config() )
