# UCR CS204 Final Review
1. Wireless Overview
	1. Link Layer Core Mission
		- Session management
			- Setup between device and infra
			- Maintain connection when moving
		- Resource Allocation
			- Devices share the channel; interference
			- Allocate time and frequency
	2. Basic wireless knowledge
		frequency, interference, hidden terminal
	3. Popular technologies: Wi-Fi and 5G
		- Bandwidth/throughput improve <- new techniques on PHY
		- Wi-Fi: Shorter range (Local area) vs. 5G: Longer range (Wide area)
		- Latest Wi-Fi applies same OFDMA resource allocation as 5G  
		- 5G supports seamless mobility; Wi-Fi usually does not  
		- Authentication method: SIM-based vs. password-based
2. Wi-Fi
	1. Architecture & Spectrum
		1. Infrastructure mode - Basic Service Set
			- Wireless host
			- Base station (called access point, AP)
		2. Spectrum divided into channels at different frequencies
			- Same channel as neighboring AP can result in interference
			- Even different channels might have overlapping frequency
	2. Wi-Fi Evolution
		1. Wi-Fi 1 (802.11b): CSMA/CA + RTS/CTS(Hidden Terminal) + 2.4GHz
		2. Wi-Fi 2 (802.11a): Orthogonal Frequency Division Multiplexing(OFDM)+5GHz
		3. Wi-Fi 3 (802.11g): OFDM + 2.4GHz
		4. Wi-Fi 4 (802.11n): Multiple-Input and Multiple-Output (MIMO) - multiple antennas transmit data simultaneously on the same frequency, use signal processing to mitigate interference and increase throughput + 2.4G & 5GHz
		5. Wi-Fi 5 (802.11ac): MU-MIMO - multiple devices share MIMO channel + 5GHz
		6. Wi-Fi 6 (802.11ax): OFDMA + 2.4GHz & 5GHz & 6GHz
		7. Wi-Fi 7 (802.11be): Multi-Link Operation
	3. Connection Setup & Scanning
		1. Probing -> Authentication -> Association
		2. Passive Scanning: Less interference and energy
			1. Send & listen Beacon periodically from AP to Host
			2. Authentication
			3. Association Request from Host to selected AP
			4. Association Response from selected AP to Host
		3. Active Scanning (probe request): Less latency for fast discovery
			1. Probe Request broadcast from Host
			2. Probe Response from APs
			3. Authentication
			4. Association Request from H1 to selected AP
			5. Association Response back
		4. Flexibility for security: can only send beacons upon request
	4. 2.4GHz vs 5GHz: both unlicensed
		1. 2.4GHz: cheaper; better range, less attenuation; backward compatible
		2. 5GHz: cleaner, higher bandwidth
	5. OFDM: OFDMA vs CSMA/CA
		1. CSMA/CA: user occupies whole channel
		2. OFDM: divide channel using frequency
		3. OFDMA: AP schedules resources regarding resource unit, in a channel under certain frequency; Higher efficiency together w/ MU-MIMO
3. Cellular Evolution
	1. Evolution: 1G and 2G only has circuit switching(exclusive, no Internet), 2.5G introduces packet switching(sharing, Internet), 3G has both, 4G/5G have packet switching only
	2. Make phone calls in 4G/5G: VoLTE or switch back
	3. Deployment
		1. Cell structure: ensure coverage
		2. Frequency re-use: if there is no overlapping area between 2 base station
	4. The architecture for all generations are similar
		1. SIM(Subscriber Identity Module) & User devices
		2. Base station: covering local areas with hardware and control
		3. Core network:  serves a region that connects to multiple base stations, manages user mobility and connection
4. 5G
	1. Schemes
		1. OFDMA-based Allocation
		2. Carrier Aggregation: link to multiple base station simultaneously
		3. MIMO: same frequency, multiple streams
	2. Allocation & Session Setup
		1. Host: Choose a PLMN, measure & select a cell (signal/policy)
		2. Uplink Requesting: Connection with BS first with random access, no allocation (OFDMA) available at this point.
		3. PRACH channel
			1. Achieve synchronization both time and frequency
			2. Get resources for connection setup message
		4. Radio Resource Control (RRC): a control-plane protocol that manages the communication between device and base station
		5. Connection with BS before Authentication (with core), can be bi-directional
		6. Data plane transmission: OFDMA
	3. Mobility: Seamless Handover (Change serving base station)
		1. Forward unsent data to new cell: ensure no packet is dropped
		2. Notify core without re-attaching: avoid setup(attach & authentication) latency
		3. Measurement selected Cells: save energy
		4. Report events when conditions are met: BS runs interna decision logic
	4. Transmission Failure
		1. HARQ retransmission: Fast retransmission to tackle bad radio
		2. RLC retransmission: Reliable retransmission to ensure no 5G packet loss 
		3. Roughly: try HARQ retransmission first; if fail, use RLC reliable retransmission
		4. Application also has retransmission mechanism
5. IoT
	1. Major idea: Sleep mode
		- Keep the device in idle without listening to the data
		- As a result, certain synchronization methods are needed
	2. Short-range communication: Bluetooth, BLE, Zigbee, Wi-Fi (fast)
		1. BT - Piconet: Ad-hoc without infra support, Master-Client structure, taking turns for multiple access control
		2. BT - Parked mode: clients “go to sleep” (park) until next master beacon
		3. BLE - Deeper sleep mode: A device can go to sleep until data is pending, unlike parked devices that continuously listens to broadcast from Master
	3. Long-range communication: Cellular (fast), NB-IoT, Long Range, Wi-Fi Halow
		1. 5G - Discontinuous reception (DRX): monitor a subframe per DRX cycle; receiver sleeps in other sub-frames
		2. NB-IoT - eDRX (Extended Discontinuous Reception)
		3. NB-IoT - Power Saving Mode (PSM): Ignores any incoming scheduling or outgoing data, wake up periodically to check for messages
		4. LoRa - normally in sleep mode: do NOT actively listen, instead, wake up and send uplink on demand, then opens a short time period after sending uplink, waiting for any downlink message
	4. Low throughput as a tradeoff for lower energy consumptions
6. DCN
	1. Traffic Patterns
		1. North-South Traffic: Partition-Aggregate, Client-Server
		2. East-West Traffic: Map Reduce, Intra-Cluster
		3. Ideal Goal Full access link rate
	2. K-ary Fat Tree
		1. Core Layer: $(k/2)^2$ core switches, each connect to $k$ pods (all aggr. switches)
		2. Each pod consists of $(k/2)^2$ servers & 2 layers of $k/2$ k-port switches
			1. Aggregation Layer: Each connects to $k/2$ edge & $k/2$ core switches
			2. Edge Layer: Each  connects to $k/2$ servers & $k/2$ aggr. switches
		3. Fully connected between aggr. layer and edge layer, but only $1/k$ partition above or below
	3. Properties
		- Identical bandwidth at any bisections
		- Cheap devices with uniform capacity
		- Great scalability: supports $k^3/4$ servers
	4. Special Addressing and Routing Scheme
		1. Problem
			- Layer 2 switch algorithm: data plane flooding!
			- Layer 3 IP routing: wasting Path Diversity!
			- Equal-Cost Multi-Path routing: Out of Order!
		2. Solution
			1. Special Addressing Scheme: IP = x.podID.switchID.hostID
			2. Two-level Lookups
				1. Prefix Lookup: Downstream
				2. Suffix Lookup: Upstream. Hash: LB & maintains packet ordering
7. Satellite
	1. GEO (Geostationary Earth Orbit): Satellite TV; Weather monitoring
		- High transmit power needed with high latency
		- 3 Satellites can cover the earth
		- Complete rotation exactly one day, synchronous to earth rotation, no adjusting necessary for device on earth
		- Large coverage making it difficult to reuse frequencies
	2. MEO (Medium Earth Orbit): Navigation (Galileo System, GPS)
		- Moderate, 6 Satellites can cover the earth
	3. LEO (Low Earth Orbit): Mobile Broadband (StarLink)
		- Fast and energy saving, need hundreds or (many) more
8. Security
	1. Four Major Security Requirements
		1. Confidentiality: Message secrecy, usually achieved by encryption
		2. Authentication: Confirm the identity of the users
			1. Challenge-Response
				1. Send a challenge over the air
				2. Only the legitimate key owner can correctly send the response based on challenge
			2. Mutual authentication starting 3G: fake base stations (FBS) in 2G
		3. Data Integrity: A message is sent unchanged from original sender
		4. Authorization: Specifying access rights to resources
9. NFV
	1. Traditional Network: middleboxes & dedicated hardware, ASIC (speed, scalability)
	2. New view: network functions & software, CPU (flexibility)
	3. Key Benefits of NFV: programmability, deployment and dynamic management
	4. Complementary technology: SDN about “control” and NFV is for implementation
	5. State Management: State stored centralized and the NFV can be stateless
	6. Performance Guarantee: Memory Isolation, Packet Isolation, Performance Isolation
10. ML for Wireless
	1. Pros
		1. Automatic solution generation
		2. Handling Complex and High Dimensional
		3. Handling Highly Dynamic Environment
		4. Add predictive capabilities
	2. Cons
		1. Data Dependencies
		2. Generalization Issues
		3. High Overhead
		4. Lack of Interpretability
		5. Security Risks
	3. Which tasks would be suitable for ML?
		1. Does the networking design work?
			1. Avoid using AI to directly perform any tasks that ensure correctness: protocol design
			2. Use ML to facilitate design and implementation
		2. Does the networking design work well?
			1. Where ML prevails – worst case scenario, cannot achieve the optimal performance: error correction, interference management, signal detection, resource allocation, traffic classification and prediction
		3. Does the networking work well consistently?
			1. Where ML might work – but be careful, attackers might target your ML model to deceive your networks: 
			2. Failure prediction: Train a model to predict incoming node failures given the high-dimensional data
			3. Traffic identification and anomaly detection: Find traffic that are not not following a specific pattern or rules
