h=[1,2,3,4,2,3,5]
b=[2,3,5,6]

class String
 def to_bytearray
  array=[]
  self.each_byte {|x| array << x}
  array
 end
end

class HexInspectorFile
 def initialize(string)
  @data=string.to_bytearray
  generate_cross()
 end
 
 def generate_cross()
   @cross={}

   @data.each_index do |x|
     j=@data[x]
     @cross[j]=[] unless @cross.include? j
     @cross[j] << x
   end
 end

 def find_substr(str,offset=0)
   len=0
   options=nil
   (offset..str.size-1).each do |x|
     char=str[x]
     if @cross.include? char
       if options==nil
         options=@cross[char].dup
         
         # Remove the current index
         options.delete(x) if str==self
       else
         options_shadow = options.dup
         puts options.inspect
         options.each_index do |y|
#          puts "a . %s %s %s" % [y,char, options[y].inspect]
          options.delete_at(y) unless @cross[char].include? options[y]+x
         end
         if options.size == 0
           options = options_shadow
           puts "Breaking"
           break
         end
       end
#       puts "%s %s" % [char, @cross[char].inspect]
     end
#     puts "Options : %s" % options.inspect
     len=x
   end

   puts "Options : len %i : %s" % [len,options.inspect] 
   #options.each do |x|
   #  puts @data[x..x+len].inspect
   #end
   return [options, len]
 end

 def method_missing(name, *args)
   # Relay unknown messages to the underlying array.
   @data.send(name, *args) 
 end
 
end

a=HexInspectorFile.new("abcdabce1234")
puts a.inspect
a.find_substr("abcdef".to_bytearray)
a.find_substr("abc".to_bytearray)
