require 'test/unit'
require 'hexfile.rb'

class SubstrTest < Test::Unit::TestCase
  def setup
    @file1=HexInspectorFile.new("123456789-12456789")
  end

  def test_short
    (matches,len) = @file1.find_substr(HexInspectorFile.new("12"))
    assert_equal(matches.size,2)
    assert_equal(matches,[0,10])
  end

  def test_nomatch
    (matches,len) = @file1.find_substr(HexInspectorFile.new("abc"))
    assert_nil(matches)

    # Test the case of an empty substr
    (matches,len) = @file1.find_substr(HexInspectorFile.new(""))
    assert_nil(matches)
  end

  def test_singlematch
    (matches,len) = @file1.find_substr(HexInspectorFile.new("124"))
    assert_equal(matches.size,1)
    assert_equal(matches,[10])
  end

  def test_selfmatch
    (matches,len) = @file1.find_substr(@file1)
    puts "Self match %s %i" % [matches.inspect, len]
  end
end
