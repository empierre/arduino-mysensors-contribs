#!/usr/bin/perl -w
#  License: Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0)
use warnings;
use strict;
use POSIX qw(strftime ceil floor);
use Device::SerialPort;
use IO::Handle;
use DateTime;
use Scalar::Util qw(looks_like_number);
use DBI;
use Config::Simple;

# Initialization strings
my $base="/home/in/";
my $conf="/home/in/.conf-mysensors";
my $ccnt;
my $cfg;
my ($count, $string, $radioId, $value);
my $Config=&read_conf($conf);
my $port = $Config->{"Internal.usb_port"};
my $domo_ip = $Config->{"Internal.domo_ip"};
my $domo_port = $Config->{"Internal.domo_port"};
my $hardware_id = $Config->{"Internal.hardware_domo"};
my $dbh;
my %sensor_tab;

# SQLite database connexion
if (! -e "mysensors.db") {
	$dbh=&init_database();
	&create_table($dbh);
} else {$dbh=&init_database();}

#SQLite sensors init
my $sth = $dbh->prepare( "SELECT * FROM sensors" );  
$sth->execute();
my $row;
while($row = $sth->fetchrow_hashref()) {
    my $id=$row->{id};
    my $subType=$row->{subtype};
    my $value=$row->{value};
    $sensor_tab{$id}->{$subType}=$value;
    print "Read $id $subType $value\n";
}
	
# USB port opening
my $ob = &connect_usb($port);
$ob->close || warn "close failed";;
$ob = &connect_usb($port);
my $sleep = 5;
print "Sleeping $sleep second to let arduino get ready...\n";
sleep $sleep;

# Now parse output
my @vals;
open(FIC,">>".$base."log-gw.txt")||die $!;
print FIC "Starting\n";
FIC->autoflush(1);
while(1) {
       $ccnt++;
       $ob->lastline("\n");

       ($count, $string) = $ob->read(255);

       #print "$ccnt $string\n\n";
       @vals  = split("\n", $string);
       foreach (@vals) {
               $_=~ s/\t/\=/;
               $_=~ s/\r//;
               $_=~ s/\n//;
                my ($radioId,$childId,$messageType,$ack,$subType,$payload) = split(";", $_);
                if (! $childId) {$childId="0";}
                if (! $messageType) {$messageType="0";}
                if (! $subType) {$subType="0";}
                if (! $payload) {$payload="0";}
                next unless ($radioId);
                next if (! looks_like_number $radioId);
                #$value = 0 unless ($value);
                #$value = $value/1000 if $radioId =~ /I/g;
                my $dt = DateTime->now(time_zone=>'local');
                my $date=join ' ', $dt->ymd, $dt->hms;
                print "$date $radioId $childId $messageType $ack $subType $payload\n";
                if ($radioId>=0) {
                        print FIC "$date $radioId $childId $messageType $ack $subType $payload\n";
                }
                if (($messageType==3)&&($subType==3)) {
			#Answer the node ID
                        my $msg = "$radioId;$childId;3;0;4;9\n";
                        my $co = $ob->write($msg);
                        warn "write failed\n" unless ($co);
                        print "$date W ($co) : $msg \n";
                        print FIC "$date W : $msg \n";
                        $ob->write_drain;
                }
                if (($messageType==2)&&($subType==24)) {
			#Answer the node VAR_1
			my $msg;
			my $val=$sensor_tab{$radioId}->{$subType}||36890;
	                $msg = "$radioId;$childId;0;3;24;$val\n";
                        my $co = $ob->write($msg);
                        warn "write failed\n" unless ($co);
                        print "$date W ($co) : $msg \n";
                        print FIC "$date W : $msg \n";
                        $ob->write_drain;
		}
                if (($messageType==4)&&($subType==13)) {
			#Answer we are Metric
                        my $msg = "$radioId;$childId;0;4;13;M\n";
                        my $co = $ob->write($msg);
                        warn "write failed\n" unless ($co);
                        print "$date W ($co) : $msg \n";
                        print FIC "$date W : $msg \n";
                        $ob->write_drain;
		}
                if (($messageType==1)&&($subType==0)) {
			# Read the Temp value
			$sensor_tab{$radioId}->{$subType}=$payload;
			&update_or_insert($radioId,$subType,$payload);
			my $hum=$sensor_tab{$radioId}->{1}||0;
			next if ($hum<=0);
			print "sending to DZ 164 $payload $hum\n";
			`curl -s "http://$domo_ip:$domo_port/json.htm?type=command&param=udevice&idx=164&svalue=$payload;$hum;2" &`;

		}
                if (($messageType==1)&&($subType==1)) {
			# Read the Humidity value
			$sensor_tab{$radioId}->{$subType}=$payload;
			&update_or_insert($radioId,$subType,$payload);
			my $temp=$sensor_tab{$radioId}->{0}||0;
			next if ($temp<=0);
			print "sending to DZ 164 $payload\n";
			`curl -s "http://$domo_ip:$domo_port/json.htm?type=command&param=udevice&idx=164&svalue=$temp;$payload;2" &`;
		}
                if (($messageType==1)&&($subType==16)) {
			# Read the leaf wetness 
			$sensor_tab{$radioId}->{$subType}=$payload;
			&update_or_insert($radioId,$subType,$payload);
			my $val=100-($payload);my $h=1;
			if ($val>50) {$h=2} elsif ($val<25) {$h=0} else {$h=1};
			print "sending to DZ 243 $val\n";
			`curl -s "http://$domo_ip:$domo_port/json.htm?type=command&param=udevice&idx=243&nvalue=$val&svalue=$val;$h" &`;
		}
 		if (($messageType==1)&&($subType==24)) {
			# AqCO2
			$sensor_tab{$radioId}->{$subType}=$payload;
			&update_or_insert($radioId,$subType,$payload);
			if ($radioId==7) {
				print "sending to DZ 124 $payload\n";
				`curl -s "http://$domo_ip:$domo_port/json.htm?type=command&param=udevice&idx=124&nvalue=$payload" &`;
			}
		}
 		if (($messageType==1)&&($subType==37)) {
			# save a DUST_LEVEL
			$sensor_tab{$radioId}->{$subType}=$payload;
			&update_or_insert($radioId,$subType,$payload);
			if ($radioId==3) {
				if ($childId==0) {#PM10
					print "sending to DZ 244 $payload\n";
					`curl -s "http://$domo_ip:$domo_port/json.htm?type=command&param=udevice&idx=244&nvalue=$payload" &`;

				} elsif ($childId=1) {#PM25
					print "sending to DZ 245 $payload\n";
					`curl -s "http://$domo_ip:$domo_port/json.htm?type=command&param=udevice&idx=245&nvalue=$payload" &`;

				}
			}
			if ($radioId==6) {
				if ($childId==0) {#PM10
					print "sending to DZ 208 $payload\n";
					`curl -s "http://$domo_ip:$domo_port/json.htm?type=command&param=udevice&idx=208&nvalue=$payload" &`;

				} elsif ($childId=1) {#PM25
					print "sending to DZ 225 $payload\n";
					`curl -s "http://$domo_ip:$domo_port/json.htm?type=command&param=udevice&idx=225&nvalue=$payload" &`;

				}
			}
		}
 		if (($messageType==1)&&($subType>=40)) {
			# save a VAR
			$sensor_tab{$radioId}->{$subType}=$payload;
			&update_or_insert($radioId,$subType,$payload);
			if ($radioId==8) {
				if ($subType==40) {#Smoke
					print "sending $payload to DZ 223 $payload\n";
					`curl -s "http://$domo_ip:$domo_port/json.htm?type=command&param=udevice&idx=223&nvalue=$payload" &`;
				} elsif ($subType==41) {#LPG
					print "sending $payload to DZ 226 $payload\n";
					`curl -s "http://$domo_ip:$domo_port/json.htm?type=command&param=udevice&idx=226&nvalue=$payload" &`;
				} elsif ($subType==42) {#O3
					print "sending $payload to DZ 210 $payload\n";
					`curl -s "http://$domo_ip:$domo_port/json.htm?type=command&param=udevice&idx=210&nvalue=$payload" &`;
				} elsif ($subType==43) {#H2
					#print "sending $payload to DZ 235 $payload\n";
					#`curl -s "http://$domo_ip:$domo_port/json.htm?type=command&param=udevice&idx=235&nvalue=$payload" &`;
				} elsif ($subType==44) {#CO
					print "sending $payload to DZ 209 $payload\n";
					`curl -s "http://$domo_ip:$domo_port/json.htm?type=command&param=udevice&idx=209&nvalue=$payload" &`;
				} elsif ($subType==45) {#PM10
					#print "sending $payload to DZ 208 $payload\n";
					#`curl -s "http://$domo_ip:$domo_port/json.htm?type=command&param=udevice&idx=208&nvalue=$payload" &`;
				} elsif ($subType==46) {#SO2
					print "sending $payload to DZ 224 $payload\n";
					`curl -s "http://$domo_ip:$domo_port/json.htm?type=command&param=udevice&idx=224&nvalue=$payload" &`;
				}
			}
		}
 		if (($messageType==1)&&($subType==4)) {
			# save a BARO
			$sensor_tab{$radioId}->{$subType}=$payload;
			&update_or_insert($radioId,$subType,$payload);
			print "sending to DZ 198 $payload\n";
			`curl -s "http://$domo_ip:$domo_port/json.htm?type=command&param=udevice&idx=198&svalue=$payload" &`;
		}
 		if (($messageType==1)&&($subType==35)) {
			# Watermeter
			$sensor_tab{$radioId}->{$subType}=$payload;
			&update_or_insert($radioId,$subType,$payload);
			my $hum=$sensor_tab{$radioId}->{1}||0;
			if ($radioId==2) {
				$payload=$payload*1000;
				print "sending to DZ 129 $payload\n";
				`curl -s "http://$domo_ip:$domo_port/json.htm?type=command&param=udevice&idx=129&svalue=$payload" &`;
			} else {
				print "What to do ?\n";
			}
		}
        }
        sleep(1);


}
close(FIC);
$ob->write_drain;
$ob->close;
undef $ob;

sub connect_usb {
	my $port=$_[0];
	Device::SerialPort->new($port, 1) || die "Can't open $port: $ +!"; 
	$ob = Device::SerialPort->new($port, 1) || die "Can't open $port: $ +!";
	$ob->databits(8);
	$ob->baudrate(115200);
	$ob->parity("none");
	$ob->stopbits(1);
	$ob->buffers( 4096, 4096 );
	$ob->write_settings();
	return $ob;
}

sub read_conf {
	my $conf=$_[0];
	$cfg = new Config::Simple($conf) or die Config::Simple->error();;
	$cfg->autosave(1);
	# getting the values as a hash:
	my %Config = $cfg->vars();
	return \%Config;
	#$user = $cfg->param("mysql.user")
	#$cfg->param("User", "sherzodR");
	#$cfg->delete('mysql.user'); 
}
sub save_conf {
	$cfg=$_[0];
	$cfg->save();
}
sub init_database {
	my $dbh = DBI->connect(          
	    "dbi:SQLite:dbname=mysensors.db", 
	    "",                          
	    "",                          
	    { RaiseError => 1 },         
	) or die $DBI::errstr;
	return $dbh;
}
sub create_table {
	#Last sensors values table
	my $stmt = qq(CREATE TABLE sensors
	      (id INT NOT NULL,
	       subtype         REAL,
	       value         REAL
		););
	my $rv = $dbh->do($stmt);
	if($rv < 0){
	   print $DBI::errstr;
	} else {
	   print "Table created successfully\n";
	}
	#Sensors type table
	$stmt = qq(CREATE TABLE hardware 
	      (id INT PRIMARY KEY NOT NULL,
		I_BATTERY_LEVEL	REAL,
		I_RELAY_NODE	INT,
		I_SKETCH_NAME	CHAR,
		I_SKETCH_VERSION REAL
		););
	$rv = $dbh->do($stmt);
	if($rv < 0){
	   print $DBI::errstr;
	} else {
	   print "Table created successfully\n";
	}
	#device table
	$stmt = qq(CREATE TABLE device 
	      (id INT PRIMARY KEY NOT NULL,
		hwid INT NOT NULL,
		sensorType INT,
		library CHAR
		););
	$rv = $dbh->do($stmt);
	if($rv < 0){
	   print $DBI::errstr;
	} else {
	   print "Table created successfully\n";
	}
}
sub insert_sensor {
	my $stmt = qq(INSERT INTO sensors (id,subtype,value)
	      VALUES ($_[0], $_[1], $_[2] ));
	my $rv = $dbh->do($stmt) or die $DBI::errstr;
	$sth->finish();
}
sub update_sensor {
	my $stmt = qq(UPDATE sensors set value=$_[2] where id=$_[0] and subtype=$_[1] );
	my $rv = $dbh->do($stmt) or die $DBI::errstr;
	$sth->finish();
}
sub update_or_insert {
	my $sth = $dbh->prepare( "SELECT value FROM sensors WHERE id=$_[0] AND subtype=$_[1]");
	$sth->execute();
	my $row = $sth->fetch;
	if (!$row) {
		insert_sensor($_[0],$_[1],$_[2]);
	} else {
		update_sensor($_[0],$_[1],$_[2]); 
	}
}
