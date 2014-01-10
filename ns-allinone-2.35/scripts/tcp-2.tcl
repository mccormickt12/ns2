set ns [new Simulator]

set nf [open out.nam w]
$ns namtrace-all $nf

set tcp [new Agent/TCP/Reno]
$tcp set window_ 10000
# $tcp set maxcwnd_ 500000
$tcp set packetSize_ 1460
$tcp set windowInit_ 4
# $tcp set max_ssthresh_ 9900000

set n0 [$ns node]
$ns attach-agent $n0 $tcp

set ftp [new Application/FTP]

# send and receive buffer 2gb
# remove cap for 
# delack count 1
$ftp attach-agent $tcp

set null0 [new Agent/TCPSink]
$null0 set total_ 1048576

set n1 [$ns node]
$ns attach-agent $n1 $null0

$ns connect $tcp $null0 

$ns duplex-link $n0 $n1 1Gb 20ms DropTail
$ns queue-limit $n0 $n1 10000


Application/FTP instproc start {} {
	[$self agent] send 10
}

$ns at 0 "$ftp start"


proc record {} {
	global ns nf null0 tcp
	set w [$tcp set cwnd_]
	set ss [$tcp set ssthresh_]
	set b [$null0 set bytes_]
	set t [$null0 set total_]
	set now [$ns now]
	puts "$w"
	puts "$ss"
	puts "$b"
	puts "$t"
	puts "$now"

}

proc finish {} {
	global ns nf
	$ns flush-trace
	close $nf
	exec nam out.nam &
	exit 0
}

$ns at 5 "finish"
$ns run

