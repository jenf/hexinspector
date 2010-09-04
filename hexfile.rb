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
 def initialize(string, generate_hash=true)
  @data=string
  generate_cross()
  @hash_width=128 # bytes
  @buzhashes = generate_buzhash(@hash_width) if generate_hash

 end
 
 def generate_buzhash(hash_width)
  # This is far far to slow
  buzhashes={}
  puts 'Generating hashes'
  (0..@data.size-1).step(hash_width) {|x|
   str=@data[x..x+hash_width-1]
   #dump_hex(str)
   hash = BuzHash.buzhash(str)
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
