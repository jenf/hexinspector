require 'hexfile.rb'
h=open(ARGV[0])
j=h.read()
HexInspectorFile.new(j, true)
