require "hexfile.rb"

h=open(ARGV[0])
file=HexInspectorFile.new(h.read)

def dump_hex(file,offset=0,width=16)
  str=""
  i=0
  (offset..file.size).step(width) do |x|
    print "%08x " % x
    (0..width-1).each do |y|
      print " " if (y % 8)==0
      print "%02x "% file[x+y] if file[x+y]!=nil
      print "  " if file[x+y]==nil
    end
    puts ""
  end
end

dump_hex(file)
