import pyshark

pkt = pyshark.FileCapture("/home/maxime/Téléchargements/test.pcapng")[0].data

print(pkt.usb_setup_wlength)

print(pkt.usb_setup_wvalue[-4:-2])

windex = hex(int(pkt.usb_setup_windex, 10))
x = windex.find("x")
print(windex[x+1:-2])

print("0x" + pkt.usb_data_fragment.replace(":", " 0x"))
