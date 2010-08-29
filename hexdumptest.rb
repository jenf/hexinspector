require "hexfile.rb"

h=open(ARGV[0])
file=HexInspectorFile.new(h.read)

def dump_hex(file,offset=0,width=16)
  str=""
  i=0
  (offset..file.size).each do |x|
    str+="%02x" % file[x]
    if i % 16 == 0
      puts str
      str=""
    else
      str+=" "
    end
    i+=1
  end
end

dump_hex(file)
