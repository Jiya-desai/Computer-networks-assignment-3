from mininet.net import Mininet
from mininet.node import Controller, OVSSwitch
from mininet.cli import CLI
from mininet.log import setLogLevel, info
from mininet.link import TCLink
import time

def create_basic_topology():
    # Create a network with OVS switches
    net = Mininet(controller=Controller, switch=OVSSwitch, link=TCLink)
    
    # Add controller
    info('*** Adding controller\n')
    c0 = net.addController('c0')
    
    # Add switches
    info('*** Adding switches\n')
    s1 = net.addSwitch('s1')
    s2 = net.addSwitch('s2')
    s3 = net.addSwitch('s3')
    s4 = net.addSwitch('s4')
    
    # Add hosts
    info('*** Adding hosts\n')
    h1 = net.addHost('h1', ip='10.1.1.2/24')  # This will be isolated
    h2 = net.addHost('h2', ip='10.1.1.3/24')  # This will be isolated
    h3 = net.addHost('h3', ip='10.0.0.4/24')
    h4 = net.addHost('h4', ip='10.0.0.5/24')
    h5 = net.addHost('h5', ip='10.0.0.6/24')
    h6 = net.addHost('h6', ip='10.0.0.7/24')
    h7 = net.addHost('h7', ip='10.0.0.8/24')
    h8 = net.addHost('h8', ip='10.0.0.9/24')
    h9 = net.addHost('h9', ip='172.16.10.10/24')  # Will not act as NAT yet
    
    # Add links between switches with 7ms delay
    info('*** Adding switch-to-switch links with 7ms delay\n')
    net.addLink(s1, s2, cls=TCLink, delay='7ms')
    net.addLink(s2, s3, cls=TCLink, delay='7ms')
    net.addLink(s3, s4, cls=TCLink, delay='7ms')
    net.addLink(s4, s1, cls=TCLink, delay='7ms')
    net.addLink(s1, s3, cls=TCLink, delay='7ms')  # Creates a loop
    
    # Add host to switch links with 5ms delay
    info('*** Adding host-to-switch links with 5ms delay\n')
    net.addLink(h3, s2, cls=TCLink, delay='5ms')
    net.addLink(h4, s2, cls=TCLink, delay='5ms')
    net.addLink(h5, s3, cls=TCLink, delay='5ms')
    net.addLink(h6, s3, cls=TCLink, delay='5ms')
    net.addLink(h7, s4, cls=TCLink, delay='5ms')
    net.addLink(h8, s4, cls=TCLink, delay='5ms')
    net.addLink(h9, s1, cls=TCLink, delay='5ms')
    
    # Connect h1 and h2 to h9 with 5ms delay (as per Q2)
    net.addLink(h1, h9, cls=TCLink, delay='5ms')
    net.addLink(h2, h9, cls=TCLink, delay='5ms')
    
    # Start the network
    info('*** Starting network\n')
    net.build()
    c0.start()
    s1.start([c0])
    s2.start([c0])
    s3.start([c0])
    s4.start([c0])
    
    # Enable STP on all switches to handle loops
    info('*** Enabling STP on all switches\n')
    for s in [s1, s2, s3, s4]:
        s.cmd('ovs-vsctl set bridge {} stp_enable=true'.format(s.name))
    
    # Wait for STP to converge
    info('*** Waiting for STP to converge (30s)...\n')
    time.sleep(30)
    
    # Run ping tests to show failure without NAT
    run_ping_tests(net)
    
    # Start CLI
    CLI(net)
    
    # Clean up
    net.stop()

def run_ping_tests(net):
    h1 = net.get('h1')
    h2 = net.get('h2')
    h3 = net.get('h3')
    h5 = net.get('h5')
    h6 = net.get('h6')
    h8 = net.get('h8')
    
    # Test a: Communication to an external host from an internal host
    info('\n*** Test a-i: Ping to h5 from h1 (Will fail without NAT)\n')
    output = h1.cmd('ping -c 3 10.0.0.6')
    info(output)
    
    info('\n*** Test a-ii: Ping to h3 from h2 (Will fail without NAT)\n')
    output = h2.cmd('ping -c 3 10.0.0.4')
    info(output)
    
    # Test b: Communication to an internal host from an external host
    info('\n*** Test b-i: Ping to h1 from h8 (Will fail without NAT)\n')
    output = h8.cmd('ping -c 3 10.1.1.2')
    info(output)
    
    info('\n*** Test b-ii: Ping to h2 from h6 (Will fail without NAT)\n')
    output = h6.cmd('ping -c 3 10.1.1.3')
    info(output)
    
    # Also test connectivity among external hosts (should work)
    info('\n*** Ping between external hosts (Should work)\n')
    output = h3.cmd('ping -c 3 10.0.0.6')  # h3 to h5
    info(output)

if __name__ == '__main__':
    setLogLevel('info')
    create_basic_topology()