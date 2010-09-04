require 'hexfile.rb'
h=open(ARGV[0])
j=h.read()
src = HexInspectorFile.new(j, false)

h=open(ARGV[1])
j=h.read()
dst = HexInspectorFile.new(j, true)

diff = src.generate_diff(dst)
puts diff.inspect