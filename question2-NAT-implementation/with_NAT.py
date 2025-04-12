from mininet.net import Mininet
from mininet.node import OVSController, OVSKernelSwitch
from mininet.link import TCLink
from mininet.topo import Topo
from mininet.cli import CLI
import time
import os


# Define custom topology with internal hosts behind a NAT and external hosts
class NATNetworkTopology(Topo):
    def build(self):
        # Create STP-enabled switches
        swA = self.addSwitch('swA', stp=True)
        swB = self.addSwitch('swB', stp=True)
        swC = self.addSwitch('swC', stp=True)
        swD = self.addSwitch('swD', stp=True)

        # Internal hosts
        client1 = self.addHost('client1', ip='10.1.1.2/24')
        client2 = self.addHost('client2', ip='10.1.1.3/24')

        # External hosts
        ext1 = self.addHost('ext1', ip='10.0.0.4/24')
        ext2 = self.addHost('ext2', ip='10.0.0.5/24')
        ext3 = self.addHost('ext3', ip='10.0.0.6/24')
        ext4 = self.addHost('ext4', ip='10.0.0.7/24')
        ext5 = self.addHost('ext5', ip='10.0.0.8/24')
        ext6 = self.addHost('ext6', ip='10.0.0.9/24')

        # NAT router bridging internal and external traffic
        nat_router = self.addHost('nat_router')

        # Internal hosts connect to NAT
        self.addLink(client1, nat_router, delay='5ms')
        self.addLink(client2, nat_router, delay='5ms')

        # External side connections
        self.addLink(nat_router, swA, delay='5ms')
        self.addLink(ext2, swB, delay='5ms')
        self.addLink(ext1, swB, delay='5ms')
        self.addLink(ext4, swC, delay='5ms')
        self.addLink(ext3, swC, delay='5ms')
        self.addLink(ext5, swD, delay='5ms')
        self.addLink(ext6, swD, delay='5ms')

        # Inter-switch links
        self.addLink(swA, swB, delay='7ms')
        self.addLink(swC, swB, delay='7ms')
        self.addLink(swA, swC, delay='7ms')
        self.addLink(swA, swD, delay='7ms')
        self.addLink(swD, swC, delay='7ms')

# Function to launch and configure the network
def start_network():
    # Initialize the network
    network = Mininet(topo=NATNetworkTopology(), controller=OVSController,
                      switch=OVSKernelSwitch, link=TCLink)
    network.start()

    # Retrieve host objects
    client1, client2, nat, ext2, ext1, ext3, ext4, ext5, ext6 = network.get(
        'client1', 'client2', 'nat_router', 'ext2', 'ext1', 'ext3', 'ext4', 'ext5', 'ext6'
    )

    # Enable Spanning Tree Protocol on all switches
    for sw_name in ['swA', 'swB', 'swC', 'swD']:
        sw = network.get(sw_name)
        sw.cmd(f'ovs-vsctl set Bridge {sw_name} stp_enable=true')

    # Create a bridge interface on NAT for internal traffic
    nat.cmd('ip link add name bridge0 type bridge')
    nat.cmd('ip link set bridge0 up')
    nat.cmd('ip link set nat_router-eth0 master bridge0')  # to client1
    nat.cmd('ip link set nat_router-eth1 master bridge0')  # to client2
    nat.cmd('ip addr add 10.1.1.1/24 dev bridge0')  # NAT internal gateway IP

    # Set default routes for internal hosts to go through NAT
    client1.cmd('ip route add default via 10.1.1.1')
    client2.cmd('ip route add default via 10.1.1.1')

    # Set default route for external hosts to NAT external IP
    for ext in [ext1, ext2, ext3, ext4, ext5, ext6]:
        ext.cmd('ip route add default via 10.0.0.1')

    # Enable IP forwarding on NAT for routing between interfaces
    nat.cmd('sysctl -w net.ipv4.ip_forward=1')

    # Set up NAT using iptables (masquerading internal traffic)
    nat.cmd('iptables -t nat -F')
    nat.cmd('iptables -t nat -A POSTROUTING -s 10.1.1.0/24 -o nat_router-eth2 -j MASQUERADE')

    # Clear any previous IP addresses on NAT interfaces
    nat.cmd("ip addr flush dev nat_router-eth0")
    nat.cmd("ip addr flush dev nat_router-eth1")
    nat.cmd("ip addr flush dev nat_router-eth2")

    # Reapply bridge settings (duplicate in original, retained for parity)
    nat.cmd('ip link add name bridge0 type bridge')
    nat.cmd('ip link set bridge0 up')
    nat.cmd('ip link set nat_router-eth0 master bridge0')
    nat.cmd('ip link set nat_router-eth1 master bridge0')
    nat.cmd('ip addr add 10.1.1.1/24 dev bridge0')

    # Assign external IP to NAT
    nat.setIP('10.0.0.1/24', intf='nat_router-eth2')
    nat.cmd('ip addr add 172.16.10.10/24 dev nat_router-eth2')

    # Wait for network stabilization
    print("Waiting 30 seconds for routes and NAT setup to settle...")
    time.sleep(30)

    # Run a full mesh ping test
    network.pingAll()

    # Section: Connectivity tests
    print("\n================ CONNECTIVITY TESTS ================\n")

    # Internal to external communication
    print("\na) Internal -> External communication tests\n")
    for i in range(3):
        print(f"\nTest {i+1}/3: client1 -> ext3")
        print(client1.cmd('ping -c 4 10.0.0.6'))

    for i in range(3):
        print(f"\nTest {i+1}/3: client2 -> ext1")
        print(client2.cmd('ping -c 4 10.0.0.4'))

    # External to internal communication
    print("\nb) External -> Internal communication tests\n")
    for i in range(3):
        print(f"\nTest {i+1}/3: ext6 -> client1")
        print(ext6.cmd('ping -c 4 10.1.1.2'))

    for i in range(3):
        print(f"\nTest {i+1}/3: ext4 -> client2")
        print(ext4.cmd('ping -c 4 10.1.1.3'))

    # iPerf performance tests
    print("\nc) iPerf bandwidth tests (3 rounds of 120s each)\n")

    # client1 runs iperf server, ext4 acts as client
    print("\nStarting iPerf server on client1...")
    client1.cmd('iperf3 -s -D')
    time.sleep(2)
    for i in range(3):
        print(f"\niPerf3 Round {i+1}/3: ext4 -> client1")
        print(ext4.cmd('iperf3 -c 10.1.1.2 -t 120'))
        time.sleep(5)
    client1.cmd('pkill iperf3')

    # ext6 runs iperf server, client2 acts as client
    print("\nStarting iPerf server on ext6...")
    ext6.cmd('iperf3 -s -D')
    time.sleep(2)
    for i in range(3):
        print(f"\niPerf3 Round {i+1}/3: client2 -> ext6")
        print(client2.cmd('iperf3 -c 10.0.0.9 -t 120'))
        time.sleep(5)
    ext6.cmd('pkill iperf3')

    # Stop the network
    network.stop()

# Entry point
if __name__ == '__main__':
    start_network()
