#/usr/bin/perl -wl

use strict;
use Device::SerialPort;
use Getopt::Std;

my $debug = 0;

# delay between read chunks (struct transmissions)
my $updateFreq = 250;

my $logFile = "CommInterpLog.csv";
open LOG, ">$logFile" or die $!;
print "Opened log file $logFile.\n";

my @structName;	# contains structs with numeric IDs via defines
my %structs;	# contains all information read from struct file

# @structFmt[structId][variableIndex] = 'type:name:precision'
# supported types: b=byte f=float i=int ul=unsigned long o=boolean
# supported precision (optional) 0|1=limit to 1 decimal, >1=leave # digits
my @structFmt;

#delimiter between printed values and structures
my $delim = " ";
my $logDelim = ",";

#autoflush STDOUT
local $| = 1;

our $opt_p;	# serial port path
our $opt_s = '/Users/monsterkruck/Documents/Arduino/TheCommunicator/Structs.h';	# struct file path
getopt('ps');

readStructFile($opt_s) || die "ERROR: Could not read struct file $opt_s\n";
printLogHeader();	# prints header rows of log file

# choose serial port
my $portName;
my @portList = glob("/dev/tty.usb*");
if($opt_p ne "") {
	$portName = $opt_p;
}elsif($#portList==0) {
	$portName = $portList[0];
} else {
	die("Specify port from @portList as $0 -p <port name>\n");
}

my $baudRate = 57600;
my $port = Device::SerialPort->new($portName)
|| die "Can't open $portName: $!\n";
print("Established $baudRate connection to $portName\n");

# 81N on the USB ftdi driver
$port->baudrate($baudRate); # you may change this value
$port->databits(8); # but not this and the two following
$port->parity("none");
$port->stopbits(1);

my $STALL_DEFAULT=10; # how many seconds to wait for new input

my $timeout=$STALL_DEFAULT;

$port->read_char_time(0);     # don't wait for each character
$port->read_const_time($updateFreq); # 1 second per unfulfilled "read" call

my @idString;	
my $lastString;	# used to backspace over previously printed string
my $badChecksums = 0;
my $goodChecksums = 0;
my $chars=0;
my $buffer="";

# main loop processing each serial port byte
while ($timeout>0) {
	my ($count,$saw)=$port->read(1); # will read _up to_ 255 chars
	if ($count > 0) {
		$chars+=$count;
		$buffer.=unpack("H*",$saw); # convert each byte to two hex chars
		
		if($buffer =~ /5555[0-9a-f][0-9a-f]$/) {
			#print "$chars $buffer\n+" if($debug);
			
			# process the buffer contents
			processBuffer($chars,$buffer);
			
			# reset parameters
			$timeout = $STALL_DEFAULT;
			$chars=0;
			$buffer="";
		}
	} else {
		$timeout--;
	}
}

if ($timeout==0) {
	die "Waited $STALL_DEFAULT seconds and never saw what I wanted\n";
}

close(LOG);

sub processBuffer() {
	my $chars = $_[0];
	my $bufString = $_[1];

	if($debug) {
		#print("$bufString\n");
		#$bufString = "dedf7ff9fa3bbe7f7cf7fbf2f4fd5e9e0d146f9e2827da464a57a55feedfc67cdffe000044410101080355554faaaa0400004842000010c233334b410000b442555544aaaa00d67416cb79af33c0d68b74ffdd42abf27fd54d1b9673dedf7ff9fa3bbe7f7cf7fbf2f4fd5e9e0d146f9e2827da464a57a55feedfc67cdffe000044410101080355554faaaa00d67416cb79af33c0d68b74ffdd42abf27fd54d1b9673dedf7ff9fa3bbe7f7cf7fbf2f4fd5e9e0d146f9e2827da464a57a55feedfc67cdffe000044410101080355554faaaa04000048420000204333334b410000b4425555d5aaaa00d67416cb79af33c0d68b74ffdd42abf27fd54d1b9673de";
		#print "\n+\n";
	}
	print("\n+\n") if($debug);
	# find whole struct with checksum
	while($bufString =~ m/aaaa([0-9a-f][0-9a-f])([0-9a-f]+)5555([0-9a-f][0-9a-f])/g) {
		
		my $id = $1;
		my $structBytes = $2;
		my $checksum = $3;
		
		print "\n$id $checksum $structBytes\n" if($debug);
		
		if(validateChecksum($checksum,$structBytes)) {
			$goodChecksums++;
			
			my ($string, $logString) = processStruct($id,$structBytes);
			
			# clear previous terminal print line
			print "\b" x (length($lastString));
			$idString[$id] = "$string";
			
			#build and print new string with updated info
			my $printStr = "";
			foreach my $idStr (@idString) {
				$printStr .= $idStr;
			}
			#print("$string\n") if($debug);
			$printStr .= " BC: $badChecksums GC:$goodChecksums          ";
		
			print("$printStr");
			print LOG ("$id,$logString\n");
			$lastString = $printStr;
			
		} else {
			#print("$chars -v\n");
			$badChecksums++;
		}
		
	
	}
}


sub processStruct() {
	my $id = $_[0];
	my $hexString = $_[1];
	
	if(!defined($structFmt[$id])) {
		die "ERROR: No struct format defined for id $id\n";
	}
	
	#my @types = split('',$structFmt[$id]);
	
	my $i = 0;
	my $outStr = $structName[$id] . "{";
	my $logStr = time();
	#foreach my $type (@types) {
	for(my$j=0;$j<=$#{$structFmt[$id]};$j++) {
		my ($type,$char,$prec) = split(':',$structFmt[$id][$j]);
		
		# float
		if($type eq 'f') {
			my $float = substr($hexString,$i,8);
			my $str = printFloat($float,$prec);
			$outStr .= "($char)" . $str;
			$logStr .= $logDelim . $str;
			$i+=8;
		
		# int
		} elsif ($type eq 'i') {
			my $int = substr($hexString,$i,4);
			my $str = printInt($int);
			$outStr .= "($char)" . $str;
			$logStr .= $logDelim . $str;
			$i+=4;
		
		# byte
		} elsif ($type eq 'b') {
			my $byte = substr($hexString,$i,2);
			my $str = printBoolean($byte);
			$outStr .= "($char)" . $str;
			$logStr .= $logDelim . $str;
			$i+=2;
			
		# unsigned long
		} elsif ($type eq 'ul') {
			my $ul = substr($hexString,$i,8);
			my $str = printUnsignedLong($ul);
			$outStr .= "($char)" . $str;
			$logStr .= $logDelim . $str;
			$i+=8;
			
		# boolean
		} elsif ($type eq 'o') {
			my $bool = substr($hexString,$i,2);
			my $str = printBoolean($bool);
			$outStr .= "($char)" . $str;
			$logStr .= $logDelim . $str;
			$i+=2;
			
		} else {
			die "ERROR: Unknown format type $type.\n";
		}
		$outStr .= $delim;
	}
	#return($logStr);
	#return($outStr . "}" . $delim . $delim);
	#return($outStr . "}" . $delim . $delim . ':::' . $logStr);
	return($outStr . "}" . $delim . $delim, $logStr);

}


sub validateChecksum() {
	my $xmtChecksum = $_[0];
	my $hexString = $_[1];
	
	my $calcChecksum = 0;
	
	for(my $i=0;$i<length($hexString);$i+=2) {
		my $hexChars = "0x" . substr($hexString,$i,2);		
		$calcChecksum += hex($hexChars);
		$calcChecksum %= 256;
	}
	
	return(hex($xmtChecksum)==$calcChecksum);	
}


sub printByte() {
	my $hexString = $_[0];
	$hexString = "00" . $hexString;
	return(printInt($hexString));
}


sub printBoolean() {
	my $hexString = $_[0];
	$hexString = "00" . $hexString;
	my $bool = unpack "S", pack "H*", $hexString;
	return($bool);
}


sub printInt () {
	my $hexString = $_[0];

	my $int = unpack "s", pack "H*", $hexString;
	return($int);
}


sub printUnsignedLong () {
	my $hexString = $_[0];
	my $ul = unpack "L", pack "H*", $hexString;
	return($ul);

}


sub printFloat () {
	my $hexString = $_[0];
	my $precision = $_[1];
	
	my $float = unpack "f", pack "H*", $hexString;
	if(($precision == 0) && ($float =~ /(\-?[0-9]+)\.([0-9]*)/)) {
		my $int = $1;
		my $dec = $2;
		$float = $int . '.' . substr($dec,0,1);
	} elsif(($precision > 0) && ($float =~ /(\-?[0-9]+)\.([0-9]*)/)) {
		my $int = $1;
		my $dec = $2;
		$float = $int . '.' . substr($dec,0,$precision);
	} else {
		return($float);
	}
}

sub printLogHeader() {
	foreach my $structName (keys(%structs)) {
		print LOG "hdr:" . $structs{$structName}{'id'} . $logDelim . $structName . $logDelim . "id" . $logDelim . "time";
		foreach my $structFmtStr (@{$structs{$structName}{'fmt'}}) {
			print LOG $logDelim . "$structFmtStr";
		}
		print LOG "\n";
	}
}

sub readStructFile() {
	my $file = $_[0];
	my %defines;
	
	if(!-e $file) { return(0); };
	
	open(my $fh, "<", "$file")
    or die "cannot open < $file: $!";
	
	my $struct = "";
	
	while(<$fh>) {
		#print;
		
		# find #defines that assign struct indexes
		if(/\#define\s+(\S+)\s?(\S)?/) {
			my $name = $1;
			my $value = $2;
			next if ($name eq 'STRUCTS_H');
			#print "Found define $name $value\n";
			$structName[$value] = $name;
			$defines{$name} = $value;
		}
		
		# find struct declarations
		if(/\s*struct\s+(\S+)\s*\{\s*\/?\/?\s*(\S+)?/) {
			$struct = $1;
			my $comment = $2;
			#print "Found struct $struct $comment\n";
			if(!exists($structs{$struct})) {
				$structs{$struct}{'id'} = -1;
				$structs{$struct}{'abbr'} = $comment;
			}
			while(<$fh>) {
				#print;
				if(/\}/) {
					#print "end of struct $struct\n";
					last;
				}
				parseStructLine($struct,$_);
			}
		}	
	
	
	}
	
	close($fh);
	
	# match #define names for struct index assignments to actual struct names 
	foreach my $name (keys(%defines)) {
		my $searchName = $name;
		my $matched = 0;
		$searchName =~ s/_//;
		foreach my $structName (keys(%structs)) {
			if($structName =~ /$searchName/i) {
				#print "Matched $name $structName\n";
				$matched++;
				$structs{$structName}{'id'} = $defines{$name};
				$structName[$defines{$name}] = $structName;
				last;
			}
		}
		if($matched == 0) {
			die "ERROR: Could not match define for $name to a struct definition in the structs file.\n";
		}
	}

	# copy %struct entries read from struct file over hard-coded $structFmt array
	foreach my $structName (keys(%structs)) {
		#print "Testing $structName\n";
		if(($structs{$structName}{'id'} != -1) && exists($structs{$structName}{'fmt'})) {
			my $id = $structs{$structName}{'id'};
			#print "Copying $structName id $id\n";
			@{$structFmt[$id]} = @{$structs{$structName}{'fmt'}};
		}
	}
	return(1);
}

sub parseStructLine() {
	my $structName = $_[0];
	my $line = $_[1];
	
	/\s*(\S+)\s+(\S+)\;\s*\/?\/?\s*(\S+)?/;
	my $testType = $1;
	my $testName = $2;
	my $testComment = $3;
	if(exists($structs{$testType})) {
		#print "Found $testType struct $testName $structs{$testType}{'abbr'}\n";
		foreach my $i (0..$#{$structs{$testType}{'fmt'}}) {
			my @list = split(':',$structs{$testType}{'fmt'}[$i]);
			my $pushString = ($#list == 2) ? "$list[0]:${structs{$testType}{'abbr'}}${testComment}.${list[1]}:${list[2]}" : "$list[0]:${structs{$testType}{'abbr'}}${testComment}.$list[1]";
			push(@{$structs{$structName}{'fmt'}},$pushString);
		}
		return();
	}
	
	if($line =~ /\s*bool\S*\s+(\S+)\;\s*\/?\/?\s*(\S+)?/) {
		my $bool = $1;
		my $comment = $2;

		pushFormat($structName, 'o', $bool, $comment);
		
		#print "Found boolean $bool $comment\n";
		return();
	}
	
	if($line =~ /\s*float\s+(\S+)\;\s*\/?\/?\s*(\S+)?/) {
		my $float = $1;
		my $comment = $2;
		
		pushFormat($structName, 'f', $float, $comment);
		
		#print "Found float $float $comment\n";
		return();
	}
	
	if($line =~ /\s*unsigned long\s+(\S+)\;\s*\/?\/?\s*(\S+)?/) {
		my $unsignedLong = $1;
		my $comment = $2;
		
		pushFormat($structName, 'ul', $unsignedLong, $comment);

		#print "Found unsigned long $unsignedLong $comment\n";
		return();
	}
	
	if($line =~ /\s*int\s+(\S+)\;\s*\/?\/?\s*(\S+)?/) {
		my $int = $1;
		my $comment = $2;
		
		pushFormat($structName, 'i', $int, $comment);

		#print "Found int $int $comment\n";
		return();
	}
	
	if($line =~ /\s*byte\s+(\S+)\;\s*\/?\/?\s*(\S+)?/) {
		my $byte = $1;
		my $comment = $2;
		
		pushFormat($structName, 'b', $byte, $comment);
		
		#print "Found byte $byte $comment\n";
		return();
	}
}

sub pushFormat() {
	my $structName = $_[0];
	my $type = $_[1];
	my $name = $_[2];
	my $comment = $_[3];

	if($name =~ /\[([0-9]+)\]$/) {
		my $maxIdx = $1 - 1;
		for my $i (0 .. $maxIdx) {
			push(@{$structs{$structName}{'fmt'}},"$type:${comment}[$i]");
			push(@{$structs{$structName}{'name'}},"$name");
		}
	} else {
		push(@{$structs{$structName}{'fmt'}},"$type:$comment");
		push(@{$structs{$structName}{'name'}},"$name");
	}
}