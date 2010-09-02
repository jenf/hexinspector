require 'hexdump.rb'

def le32(str,ptr)
  h=str[ptr]
  h+=(str[ptr+1] << 8)
  h+=(str[ptr+2] << 16)
  h+=(str[ptr+3] << 24)
  return h
end

class XBCopy
  def initialize(off, csize)
   @off=off
   @csize = csize
  end
  
  def apply_patch(src,dst)
   dst << src[@off..@off+@csize-1]
  end
end

class XBInsert
  def initialize(csize,data)
   @csize = csize
   @data = data
  end
  
  def apply_patch(src,dst)
   dst << @data
  end
end


class XBDiffFile
  def initialize(str)
    @instructions=[]
    ptr=8 # XBL_BPATCH_HDR_SIZE
    i=0
    while ptr < str.size
     puts "%i %i" % [ptr, str.size]
     i+=1
     type=case str[ptr]
       when 1 then :INS
       when 2 then :CPY
       when 3 then :INSB
     end
     
     ptr+=1
 
     puts type
 
     case type
     when :INS then
      csize = str[ptr]
      ptr+=1
      data = str[ptr..ptr+csize-1]
      ptr+=csize
      dump_hex(data)
      @instructions << XBInsert.new(csize,data)
     when :INSB then
      csize = le32(str,ptr)
      ptr+=4
      data = str[ptr..ptr+csize]
      ptr+=csize
      @instructions << XBInsert.new(csize,data)      
     when :CPY then
      off = le32(str,ptr)
      ptr+=4
      dump_hex(str[ptr..ptr+3])
      csize = le32(str,ptr) 
      ptr+=4
      @instructions << XBCopy.new(off,csize)
     end
    end
  end
  
  def apply_patch(file)
   dst = ""
   src_ptr = 0
   @instructions.each do |x|
    x.apply_patch(file,dst)
   end
   dst
  end
end

h=open(ARGV[0])
bin=h.read()
file=XBDiffFile.new(bin)
puts file.inspect

h=open(ARGV[1])
src=h.read()
j=file.apply_patch(src)
dump_hex(j)
