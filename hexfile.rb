require 'BuzHash.rb'
require 'hexdump.rb'

class String
 def to_bytearray
  array=[]
  self.each_byte {|x| array << x}
  array
 end
end

class HexInspectorFile
 attr :buzhashes
 def initialize(string, generate_hash=false)
  @data=string
  generate_cross()
  @hash_width=128 # bytes
  @buzhashes = generate_buzhash(@hash_width) if generate_hash

 end
 
 def generate_diff(dst)
  raise 'Dst does not have a hash table' if nil == dst.buzhashes
  diff=[]
  srcptr=0
  dstptr=0
  srcsize=self.size
  dstsize=dst.size
  buzhash = 0
  
  segment_dststart=dstptr
  segment_srcstart=srcptr
  mode=:synced
  diffprecision=:exact

  # TODO: Check what happens on files smaller than hash_width
  raise 'Untested condition' if srcsize < @hash_width
  
  # Preload the rolling hash with the first hash_width, exclude the last letter
  buzhash = BuzHash.new(@hash_width)
  (0..@hash_width-2).each {|x|
   buzhash.add_char(@data[x])
#   puts "%i %i" %[x,buzhash.hash]
  }
  
#  puts dst.buzhashes.inspect
#  puts buzhash  
#  puts
  # Go through each character
  while srcptr < srcsize
  
   if dstptr<=dstsize
   
    # Update the hash
    if srcptr+@hash_width<srcsize 
     buzhash.add_char(@data[srcptr+@hash_width-1])
#     puts "%i %i" % [srcptr,buzhash.hash]
    else
     buzhash = nil unless srcptr+@hash_width<srcsize
    end
    
    case mode
     when :synced
     
      if @data[srcptr]==dst[dstptr]
       srcptr+=1
       dstptr+=1
      else
       puts 'Diff at %i %i' % [srcptr,dstptr]
       srclostsync = srcptr
       diff << [segment_srcstart,srcptr,segment_dststart,dstptr,diffprecision]
       mode = :unsynced_near
      end
      
     when :unsynced_near
      if (srcptr < srclostsync+16) || (srcptr+@hash_width>srcsize)
       if @data[srcptr]==dst[dstptr]
        puts "Resync at %i %i" % [srcptr,dstptr]
        mode=:synced
        diffprecision=:exact
        segment_srcstart=srcptr
        segment_dststart=dstptr
       else
        srcptr+=1
       end
      else
       mode = :unsynced_far
      end
      
     when :unsynced_far
#      puts buzhash.hash
      j = dst.buzhashes[buzhash.hash]
      if j!=nil
       # The contents of j are already sorted
       j.each {|x|
         if x>dstptr
          dstptr=x
          mode=:synced
          diffprecision=:indexed
          segment_srcstart=srcptr
          segment_dststart=dstptr
          puts "Sync at %i %i" % [srcptr,dstptr]
          break
         end
       }
      end
      if mode == :unsynced_far
       srcptr+=1
       if (srcptr+@hash_width>srcsize)
        mode=:unsynced_near
       end
      end
      
     else
      puts 'Bad state'
      srcptr+=1
    end
  
   else
    srcptr +=1
   end
   
  end
  
  # Add the last one if we're still in sync
  if mode==:synced
   diff << [segment_srcstart,srcptr,segment_dststart,dstptr,diffprecision]
  else
   diff << [segment_srcstart,srcsize,segment_dststart,dstsize,:diff]  
  end
  
  return generate_diffhunks(diff)
 end
 
 def generate_diffhunks(diff)
  difftoadd=[]
  last=diff[0]
  diff.each {|x|
    if x!=last
      difftoadd << [last[1]+1,x[1]-1,last[3]+1,x[3]-1,:diff]
    end
    last = x
  }
  return diff.concat(difftoadd).sort
 end

 def generate_buzhash(hash_width)
  # This is far far to slow
  buzhashes={}
  puts 'Generating hashes'
  (0..@data.size-1).step(hash_width) {|x|
   str=@data[x..x+hash_width-1]
   #dump_hex(str)
   hash = BuzHash.buzhash(str,0)
   puts "%i %i" % [x,hash]
   #puts x
   #puts hash
   
   buzhashes[hash]=[] unless buzhashes.include? hash
   buzhashes[hash] << x
  }
  puts 'Generating hashes complete'
  return buzhashes
 end
 
 def generate_cross()
   @cross={}

   (0..@data.size).each do |x|
     j=@data[x]
     @cross[j]=[] unless @cross.include? j
     @cross[j] << x
   end
 end

 def find_substr(str,offset=0, minimum=4, stopat=32)
   len=0
   options=nil
   (offset..(str.size-1)).each do |x|
     char=str[x]
     if @cross.include? char
       if options==nil
         options=@cross[char].dup
         
         # Remove the current index
         options.delete(x) if str==self
       else
         options_shadow = options.dup
         #puts options.inspect
         options.each_index do |y|
#          puts "a . %s %s %s" % [y,char, options[y].inspect]
          options.delete_at(y) unless @cross[char].include? options[y]+x
         end
         if options.size == 0
           options = options_shadow
#           puts "Breaking"
           break
         end
       end
#       puts "%s %s" % [char, @cross[char].inspect]
     end
#     puts "Options : %s" % options.inspect
     len=x-offset
     break if len>stopat
   end

   puts "Out!"
#   puts "Options : len %i : %s" % [len,options.inspect] 
   #options.each do |x|
   #  puts @data[x..x+len].inspect
   #end
   return [nil, len] if len<minimum
   return [options, len]
 end

 def method_missing(name, *args)
   # Relay unknown messages to the underlying array.
   @data.send(name, *args) 
 end
 
end
