#!/usr/bin/perl
$file = "/sys/devices/w1_bus_master1/28-0014521b65ff/rw";
$ip = "192.168.1.86";
$pt = "5665";
$id = "410";

sub send_command {
  my $cmd = shift;
  my $f;
  sleep(1);
  open($f, ">", $file) or die "can't open $file: $!";
  binmode($f) or die "can't binmode $file: $!";;
  syswrite($f, $cmd) or die "can't write $file: $!";
  close($f) or die "can't close $file: $!";
}

sub read_data {
  my $f;
  my $t;
  open($f, "<", $file) or die "can't open $file: $!";
  binmode($f) or die "can't binmode $file: $!";
  sysread($f, $t, 1000);
  close($f) or die "can't close $file: $!";
  return $t;
}

send_command(chr(0x44)); # Start temp conversion command
send_command(chr(0xbe)); # Read data command
my $ret = read_data();
my $tempF=unpack("s", substr($ret,0,2))/16; # Grab the first 2 bytes, shift the decimal
my $temp =sprintf "%.2f", $tempF;

use LWP::Simple;
my $url = "http://$ip:$pt/json.htm?type=command&param=udevice&idx=$id&nvalue=0&svalue=$temp";
print "$url\n";
my $content = get($url);
