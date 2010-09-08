require "hexfile.rb"
require 'rubygems'
require 'ncurses'

h=open(ARGV[0])
file=HexInspectorFile.new(h.read)

class HexPager
  attr_accessor :offset,:file
  def initialize(file,width,height, x=0, y=0)
    @file = file
    @width = width
    @bytewidth = (width-(8+2+1))/3 # (8 bytes+2 gap - width)/3 (two chars + space)
    @height = height
    @x = x
    @y = y
    @window = Ncurses::WINDOW.new(height,width, x, y)
    @offset = 0
  end

  def dump_hex(lines,offset=0,width=16,startpos=0,curpos=0)
    i=0
    (offset..offset+(lines*width)).step(width) do |x|
      if x < file.size
        str=""
        str << "%08x " % x
        (0..width-1).each do |y|
          #str << " " if (y % 8)==0
          str << "%02x "% @file[x+y] if @file[x+y]!=nil
          str << "  " if @file[x+y]==nil
        end
        @window.mvaddstr(i+startpos+1,1,str)
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
    @bytewidth = (width-(8+2+1))/3 # (8 bytes+2 gap - width)/3 (two chars + space)

    @window.move(x,y)
  end
  
  def keypress(key)
    case key
    when Ncurses::KEY_LEFT
       @offset-=1
       @offset=0 if @offset<0
       true
    when Ncurses::KEY_RIGHT
       @offset+=1
       true
     when Ncurses::KEY_UP
      @offset-=@bytewidth
      @offset=0 if @offset<0
      true
     when Ncurses::KEY_DOWN
      @offset+=@bytewidth
      true
     else
      false
    end
    
  end
  
end

class HexInspector
  def initialize(window,file)
    @window = window
    @window_size_changed = true
    @file = file
    @offset=0
    Ncurses.start_color()
    @normal_col=0
    @diff_col = 1
    @reversed_col=2
    Ncurses.init_pair(@diff_col, Ncurses::COLOR_RED, Ncurses::COLOR_BLACK);
    Ncurses.init_pair(@reversed_col, Ncurses::COLOR_BLACK, Ncurses::COLOR_WHITE);
    @height = Ncurses.LINES()
    @width = Ncurses.COLS
    @pager = HexPager.new(file,@width/2,@height-5,0, 0)
  end

  def show_ruler()
    bit8_val=@pager.file[@pager.offset]
    @window.attrset(Ncurses.COLOR_PAIR(@reversed_col))
    Ncurses.mvaddstr(@height-1,0,"Byte %i/0x%08x Val : %03i/0x%02x/0%03o Substr Match: %i" % [@pager.offset,@pager.offset,bit8_val,bit8_val,bit8_val, @matches.size])
    @window.attrset(Ncurses.COLOR_PAIR(@normal_col))
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

   while true
    if @window_size_changed
      @height=@window.getmaxy
      @pager_lines=@height-5
      @window_size_changed=false
      
    end


    @pager.draw()
#    show_substrs()
    show_ruler()

    @window.refresh
    key=@window.getch
    
    if false == @pager.keypress(key)
      case @window.getch

       when 's'[0]
         @pager.offset=@matches[0]
       when 'q'[0]
         raise 'Quit'
       when Ncurses::KEY_RESIZE
         @window_size_changed=true
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
inspector = HexInspector.new(Ncurses.stdscr, file)
begin
  inspector.main_loop
ensure
  Ncurses.curs_set(1)
  Ncurses.endwin()
end
