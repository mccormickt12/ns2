#Create a simulator object
set ns [new Simulator]

#Define different colors for data flows (for NAM)
$ns color 1 Blue
$ns color 2 Red

#Open the NAM trace file
set nf [open out.nam w]
$ns namtrace-all $nf

#Define a 'finish' procedure
proc finish {} {
        global ns nf
        $ns flush-trace
        #Close the NAM trace file
        close $nf
        #Execute NAM on the trace file
        exec nam out.nam &
        exit 0
}

#Create four nodes
set n0 [$ns node]
set n1 [$ns node]
set n2 [$ns node]
set n3 [$ns node]
set n4 [$ns node]

#Create links between the nodes
$ns duplex-link $n0 $n1 5Mb 10ms DropTail
$ns duplex-link $n1 $n2 5Mb 10ms DropTail
$ns duplex-link $n2 $n3 1Mb 20ms DropTail
$ns duplex-link $n2 $n4 2MB 15ms DropTail

#Set Queue Size of link (n2-n3) to 10
$ns queue-limit $n2 $n3 100



#Setup a TCP connection

Agent/TCP set window_ 20
Agent/TCP set packetSize_ 1460
Agent/TCP set windowInit_ 4
Agent/TCP set maxseq_ 10

set tcp [new Agent/TCP]
$tcp set fid_ 1
set tcp1 [new Agent/TCP]
$tcp1 set fid_ 2

$ns attach-agent $n0 $tcp
$ns attach-agent $n1 $tcp1
set sink [new Agent/TCPSink]
set sink1 [new Agent/TCPSink]
$ns attach-agent $n3 $sink
$ns attach-agent $n4 $sink1
$ns connect $tcp $sink
$ns connect $tcp1 $sink1


#Setup a FTP over TCP connection
set ftp [new Application/FTP]
set ftp1 [new Application/FTP]
$ftp attach-agent $tcp
$ftp1 attach-agent $tcp1 

Application/FTP instproc start {} {
	[$self agent] send 10
}


$ns at 0.2 "$ftp start"
$ns at 0.4 "$ftp1 start"
$ns at 4.0 "$ftp stop"
$ns at 4.2 "$ftp1 stop"
$ns at 5.0 "finish"



#Run the simulation
$ns run

