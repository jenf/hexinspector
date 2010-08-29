require "hexfile.rb"
require 'rubygems'
require 'ncurses'

h=open(ARGV[0])
file=HexInspectorFile.new(h.read)

def dump_hex(file,lines,offset=0,width=16)
  i=0
  (offset..offset+(lines*width)).step(width) do |x|
    str=""
    str << "%08x " % x
    (0..width-1).each do |y|
      str << " " if (y % 8)==0
      str << "%02x "% file[x+y] if file[x+y]!=nil
      str << "  " if file[x+y]==nil
    end
    Ncurses.mvaddstr(i,0,str)
    i+=1
  end
end

line_offset=0
Ncurses.initscr
Ncurses.cbreak
Ncurses.noecho
Ncurses.keypad(Ncurses.stdscr, true)

while true
 Ncurses.clear
 width=16
 dump_hex(file,16,line_offset*width)
 Ncurses.refresh
 window = Ncurses.stdscr
 case window.getch
 when Ncurses::KEY_UP
   line_offset-=1
   line_offset=0 if line_offset<0
 when Ncurses::KEY_DOWN
   line_offset+=1
 end
end
