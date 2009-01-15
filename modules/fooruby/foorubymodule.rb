
class Foo
	def bar(msg)
		puts "bar: #{msg}"
	end
end

foo = Foo.new

puts "foo=#{foo}"

Titanium.set("fooruby", foo)

puts "ok bound, exiting script"