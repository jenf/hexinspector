require "hexfile.rb"
require 'rubygems'
require 'ncurses'

h=open(ARGV[0])
file=HexInspectorFile.new(h.read)
h.close()

file2=nil
diff = nil

if ARGV.size==2
 h=open(ARGV[1])
 file2=HexInspectorFile.new(h.read,true)
 diff = file.generate_diff(file2)
 puts diff.inspect
 h.close()
end

class HexPager
  attr_accessor :offset,:file,:otherpager
  def initialize(file,width,height, x=0, y=0, diff=nil, main=nil)
    @file = file
    @width = width
    @bytewidth = (width-(8+2+2))/3 # (8 bytes+2 gap - width)/3 (two chars + space)
    @height = height
    @x = x
    @y = y
    @window = Ncurses::WINDOW.new(height,width, y, x)
    @offset = 0
    @diff = diff
    @otherpager = main
    @mode = :src if @otherpager==nil
    @mode = :dst if @otherpager!=nil
    
    # Link both pagers
    @otherpager.otherpager=self if @otherpager!=nil
  end

  def text_color(char)
    if char>='0'[0] and char<='9'[0]
     :number
    elsif (char>='A'[0] and char<='Z'[0]) or (char>='a'[0] and char<='z'[0])
     :letter
    elsif char==255 or char<=32
     :unprintable
    else
     :normal
    end
  end
  
  def isdifferent(pos,diff,mode=:src)
    diff.each {|x|
      if mode==:src
        return x[-1]==:diff if pos>=x[0] and pos<x[1]
       elsif mode==:dst
        return x[-1]==:diff if pos>=x[2] and pos<x[3]
       end
    }
    return false
  end

  def get_diff_index(pos,diff, mode=:src)
    diff.each_index {|x|
      if mode==:src
        return x if pos>=diff[x][0] and pos<diff[x][1]
       elsif mode==:dst
        return x if pos>=diff[x][2] and pos<diff[x][3]
       end
    }
    return nil
  end

  def dump_hex(lines,offset=0,width=16,startpos=0,curpos=0)
    i=0

    (offset..offset+(lines*width)).step(width) do |x|
      if x < file.size
        str=""
        str << "%08x " % x
        ypos=0
        @window.mvaddstr(i+startpos+1,1+ypos,str)
        ypos+=str.size
        str=""
        
        (0..width-1).each do |y|
          #str << " " if (y % 8)==0
          char = @file[x+y]
          if char!=nil
            str << "%02x "% char
            if @diff!=nil
             if isdifferent(x+y,@diff,@mode)
              @window.attrset(HexInspector.get_color(:diff))
             else
              @window.attrset(HexInspector.get_color(text_color(char)))
             end
            else
             @window.attrset(HexInspector.get_color(text_color(char)))
            end
            @window.mvaddstr(i+startpos+1,1+ypos,str)
            @window.attrset(HexInspector.get_color(:normal))
            ypos+=str.size
            str =""
          end
        end

      end
      i+=1
    end
  end
  def draw()
    @window.clear()
    @window.border(*([0]*8))
    
    dump_hex(@height-3,@offset,@bytewidth)
    @window.noutrefresh()
  end
  
  def resize(width, height, x, y)
    @width = width
    @height = height
    @x = x
    @y = y
    @window.resize(height, width)
    @bytewidth = (width-(8+2+2))/3 # (8 bytes+2 gap - width)/3 (two chars + space)

    @window.mvwin(y,x)
  end
  
  def primary_keypress(key, buffer)
    case key
     when "["[0]
      if @diff
       pos=get_diff_index(@offset,@diff,@mode)
       if pos>0
        @offset=@diff[pos-1][0] if @mode==:src
        @offset=@diff[pos-1][2] if @mode==:dst
        @otherpager.offset=@diff[pos-1][2] if @mode==:src
        @otherpager.offset=@diff[pos-1][0] if @mode==:dst
       end
      end
      [true, ""]
     when "]"[0]
      if @diff
       pos=get_diff_index(@offset,@diff,@mode)
       if pos!=nil and pos+1<(@diff.size-1)
        @offset=@diff[pos+1][1] if @mode==:src
        @offset=@diff[pos+1][3] if @mode==:dst
        @otherpager.offset=@diff[pos+1][3] if @mode==:src
        @otherpager.offset=@diff[pos+1][1] if @mode==:dst
       end
      end
      [true,""]
     else
      [false, buffer]
    end
  end
  
  def keypress(key, buffer)
    case key
    when Ncurses::KEY_LEFT
       @offset-=1
       @offset=0 if @offset<0
       [true, buffer]
    when Ncurses::KEY_RIGHT
       @offset+=1
       [true, buffer]
     when Ncurses::KEY_UP
      @offset-=@bytewidth
      @offset=0 if @offset<0
      [true, buffer]
     when Ncurses::KEY_DOWN
      @offset+=@bytewidth
      [true, buffer]
     when Ncurses::KEY_NPAGE
      @offset+=@bytewidth*@height
      [true, buffer]
     when Ncurses::KEY_PPAGE
      @offset-=@bytewidth*@height
      @offset=0 if @offset<0
      [true, buffer]
     when "G"[0],"g"[0]
      @offset= buffer.to_i(0)
      @offset = @file.size-(@bytewidth*@height) if @offset==-1
      [true,""]

     else
      [false, buffer]
    end
    
  end
  
end

class HexInspector
  class << self; attr_accessor :colors end
  @colors={:normal=>0,:reversed=>2,:diff=>1,:letter=>3,:number=>4,:unprintable=>5}
  
  def self.get_color(color)
    Ncurses.COLOR_PAIR(self.colors[color])
  end
  
  def initialize(window,file, file2=nil, diff=nil)
    @window = window
    @window_size_changed = true
    @file = file
    @file2 = file2

    Ncurses.start_color()
    Ncurses.init_pair(HexInspector::colors[:diff], Ncurses::COLOR_BLACK, Ncurses::COLOR_RED);
    Ncurses.init_pair(HexInspector::colors[:reversed], Ncurses::COLOR_BLACK, Ncurses::COLOR_WHITE);
    Ncurses.init_pair(HexInspector::colors[:unprintable], Ncurses::COLOR_RED, Ncurses::COLOR_BLACK);
    Ncurses.init_pair(HexInspector::colors[:letter], Ncurses::COLOR_BLUE, Ncurses::COLOR_BLACK);
    Ncurses.init_pair(HexInspector::colors[:number], Ncurses::COLOR_GREEN, Ncurses::COLOR_BLACK);
     
    @height = Ncurses.LINES()
    @width = Ncurses.COLS
    @pager = HexPager.new(file,(@width/2)-1,@height-5,0, 0, diff) if file2!=nil
    @pager = HexPager.new(file,@width,@height-5,0, 0, diff) if file2==nil
    @pager2 = HexPager.new(file2,(@width/2)-1,@height-5,(@width/2), 0, diff, @pager) if file2!=nil
  end

  def show_ruler(buffer)
    bit8_val=@pager.file[@pager.offset]
    @window.attrset(HexInspector.get_color(:reversed))
    Ncurses.mvaddstr(@height-1,0,"Pager %s Byte %i/0x%08x Val : %03i/0x%02x/0%03o Buffer: %s" % [@pager==@primarypager?"Left":"Right",@pager.offset,@pager.offset,bit8_val,bit8_val,bit8_val, buffer])
    @window.attrset(HexInspector.get_color(:normal))
  end

  def show_substrs()
    (matches,len) = @file.find_substr(@file,@offset)
    @matches=matches
    @matches=[] if @matches==nil
    (0..(matches.size < 3 ? matches.size-1 : 3)).each do |x|
     Ncurses.mvaddstr(@height-4+x,0,"Match %i %i" % [matches[x], len])
    end
  end
  def main_loop
   @matches=[] if @matches==nil
   @primarypager=@pager
   buffer=""
   while true
    if @window_size_changed
      @height= Ncurses.LINES
      @width = Ncurses.COLS
      
      @pager.resize((@width/2)-1,@height-5,0,0)         if @pager2!=nil
      @pager2.resize((@width/2)-1,@height-5,@width/2,0) if @pager2!=nil
      @pager.resize(@width,@height-5,0,0)               if @pager2==nil

      @pager_lines=@height-5
      @window_size_changed=false
      
    end


    @pager.draw()
    @pager2.draw() if @pager2!=nil
#    show_substrs()
    show_ruler(buffer)

    @window.refresh
    key=@window.getch
    (claimed,buffer) = @primarypager.primary_keypress(key,buffer)
    if false == claimed
      @pager2.keypress(key,buffer) if @pager2!=nil
      (claimed, buffer) = @pager.keypress(key,buffer)
      if false == claimed
        case key

         when 's'[0]
           @pager.offset=@matches[0]
         when "<"[0]
           @primarypager=@pager
         when ">"[0]
           @primarypager=@pager2
         when 'q'[0]
           raise 'Quit'
         when Ncurses::KEY_RESIZE
           @window_size_changed=true
         when "\n"[0]
           buffer=""
         when 0..255
           buffer << key.chr
        end
      end
    end
   end
 end
end

line_offset=0
Ncurses.initscr
Ncurses.cbreak
Ncurses.noecho
Ncurses.keypad(Ncurses.stdscr, true)
Ncurses.stdscr.intrflush(false)
Ncurses.clear


inspector = HexInspector.new(Ncurses.stdscr, file, file2, diff)
begin
  inspector.main_loop
ensure
  Ncurses.curs_set(1)
  Ncurses.endwin()
end
