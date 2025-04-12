#!/usr/bin/python

from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import Controller
from mininet.log import setLogLevel, info
from mininet.link import TCLink
import time

class NetworkLoopsTopo(Topo):
    """
    Network topology with loops for CS331 Assignment#3 Q1
    """
    def build(self):
        # Add switches
        s1 = self.addSwitch('s1')
        s2 = self.addSwitch('s2')
        s3 = self.addSwitch('s3')
        s4 = self.addSwitch('s4')
        
        # Add hosts
        h1 = self.addHost('h1', ip='10.0.0.2/24')
        h2 = self.addHost('h2', ip='10.0.0.3/24')
        h3 = self.addHost('h3', ip='10.0.0.4/24')
        h4 = self.addHost('h4', ip='10.0.0.5/24')
        h5 = self.addHost('h5', ip='10.0.0.6/24')
        h6 = self.addHost('h6', ip='10.0.0.7/24')
        h7 = self.addHost('h7', ip='10.0.0.8/24')
        h8 = self.addHost('h8', ip='10.0.0.9/24')
        
        # Add links between switches (7ms delay)
        self.addLink(s1, s2, delay='7ms')
        self.addLink(s2, s3, delay='7ms')
        self.addLink(s3, s4, delay='7ms')
        self.addLink(s4, s1, delay='7ms')
        self.addLink(s1, s3, delay='7ms')  # Cross-link creating the loop
        
        # Add links between hosts and switches (5ms delay)
        self.addLink(h1, s1, delay='5ms')
        self.addLink(h2, s1, delay='5ms')
        self.addLink(h3, s2, delay='5ms')
        self.addLink(h4, s2, delay='5ms')
        self.addLink(h5, s3, delay='5ms')
        self.addLink(h6, s3, delay='5ms')
        self.addLink(h7, s4, delay='5ms')
        self.addLink(h8, s4, delay='5ms')

def run_ping_test(net, source, dest, output_file):
    """Run ping test between two hosts and write to output file"""
    message = f"\n--- Testing ping from {source} to {dest} ---"
    print(message)
    with open(output_file, 'a') as f:
        f.write(message + "\n")
    
    # Run ping with 3 packets
    result = net.get(source).cmd(f"ping -c 3 {net.get(dest).IP()}")
    print(result)
    
    with open(output_file, 'a') as f:
        f.write(result + "\n")
    
    return result

def main():
    """Create network and run tests"""
    # Create output file for ping results
    output_file = "q1_part_a_results.txt"
    with open(output_file, 'w') as f:
        f.write("CS331 Assignment #3 - Q1 Part A: Network with Loops\n")
        f.write("==============================================\n\n")
        f.write("Network topology with loops - no STP enabled\n\n")
    
    topo = NetworkLoopsTopo()
    net = Mininet(topo=topo, link=TCLink, controller=Controller)
    net.start()
    
    message = "Network topology created with loops."
    print(message)
    with open(output_file, 'a') as f:
        f.write(message + "\n")
    
    # Wait for network to stabilize
    message = "Waiting for network to stabilize (60 seconds)..."
    print(message)
    with open(output_file, 'a') as f:
        f.write(message + "\n")
    
    time.sleep(60)  # Wait 60 seconds for network to stabilize
    
    # Run the ping tests
    run_ping_test(net, 'h1', 'h3', output_file)
    time.sleep(60)  # Wait 30 seconds between tests as instructed
    
    run_ping_test(net, 'h5', 'h7', output_file)
    time.sleep(60)
    
    run_ping_test(net, 'h8', 'h2', output_file)
    
    message = "\nAll tests completed. Results saved to " + output_file
    print(message)
    
    # Cleanup
    net.stop()

if __name__ == '__main__':
    setLogLevel('info')
    main()