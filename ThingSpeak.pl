#!/usr/bin/perl
use v5.14;
use LWP::Simple;                # From CPAN
use JSON ;    # From CPAN
use File::Slurp;
use LWP::UserAgent;
use Crypt::SSLeay;
use Data::Dumper;               # Perl core module
use strict;                     # Good practice
use warnings;                   # Good practice
use utf8;
use Time::Piece;
use feature     qw< unicode_strings >;

my $API_KEY = '';
my $FEED = "88871";
my $feed = { 'version' => '1.0.0', 'datastreams' => [] };
 
# Create an HTTP client
my $ua = LWP::UserAgent->new;
$ua->agent('RasperryPiInMyHome/1.4 ');
 
my $trendsurl = "http://192.168.0.28:8080/json.htm?type=devices&filter=all&used=true&order=Name";

my $json = $ua->get( $trendsurl );
die "Could not get $trendsurl!" unless defined $json;
# Decode the entire JSON
my $decoded = JSON->new->utf8(0)->decode( $json->decoded_content );
my $url="";

my @results = @{ $decoded->{'result'} };
foreach my $f ( @results ) {
	#baro
	if ($f->{"idx"}==198) {
		$url.="&field1=".$f->{Pressure};
  	}
	# Ext T
	# Ext H
	if ($f->{"idx"}==422) {
		$url.="&field2=".$f->{Temp};
		$url.="&field3=".$f->{Humidity};
  	}
	# Rain
	if ($f->{"idx"}==448) {
		$url.="&field4=".$f->{Rain};
  	}
	# UV
	if ($f->{"idx"}==218) {
		$url.="&field5=".$f->{UVI};
  	}
	# lux
	if ($f->{"idx"}==361) {
		my ($data)=($f->{Data}=~/(\d+) Lux/);	
		$url.="&field6=".$data;
  	}
	# CO2
	if ($f->{"idx"}==124) {
		my ($data)=($f->{Data}=~/(\d+) ppm/);	
		$url.="&field7=".$data;
  	}
	# PM10
	if ($f->{"idx"}==244) {
		my ($data)=($f->{Data}=~/(\d+) ppm/);	
		$url.="&field8=".$data;
  	}

}

	# Create a HTTP request
	my $req = HTTP::Request->new(POST => "http://api.thingspeak.com/update?key=$API_KEY".$url);
	 
	# Make the request
	my $res = $ua->request($req);
	unless ($res->is_success) {
		print STDERR $res->status_line, "\n";
		print STDERR $res->content, "\n";
	}

