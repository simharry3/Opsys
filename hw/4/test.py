import os
import sys
import socket
import time

host = '127.0.0.1'
port = 8881

s = socket.socket()
os.system('rm CrispyChickenStrips/data.txt')
print "BEGINNING TESTING:\n"
print "====================="

s.connect((host, port))

#s.send('STORE data.txt 24\nabcdefghijlaaaaaaaaaaaa\n')
time.sleep(1)
# time.sleep(1);
# s.send('READ data.txt 4 12\n')

s.send(('STORE data.txt 1027\naaLorem ipsum dolor sit amet, consectetur adipiscing elit. Etiam eleifend malesuada aliquet. Pellentesque aliquet mattis orci, sed pellentesque dui rhoncus et. Sed venenatis, neque eu fringilla faucibus, turpis nisl efficitur lorem, eu rutrum eros metus a massa. Nunc urna ex, dapibus pellentesque consequat vestibulum, vestibulum in tellus. Curabitur at leo metus. Aenean in vehicula diam. Nulla imperdiet libero lectus, ac tincidunt orci vulputate quis. Nullam accumsan lectus placerat efficitur semper. Sed semper nec felis eu scelerisque. Donec id risus ex. Mauris ipsum eros, faucibus a egestas quis, luctus vitae diam. Praesent viverra diam sed sapien ultrices, id mollis erat convallis. Ut placerat felis vitae urna viverra, at commodo velit sagittis. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Mauris at urna in lacus finibus pharetra vitae eget mi. In quis nulla libero. Etiam dapibus, lacus at tristique dapibus, arcu orci feugiat felis, ut facilisis arcu lorem eu volutpat.'))
time.sleep(5)
s.send(('READ data.txt 0 1000\n'));