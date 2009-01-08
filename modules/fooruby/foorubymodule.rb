
class Foo
	def bar(msg)
		puts "bar: #{msg}"
	end
end

foo = Foo.new

puts "foo=#{foo}"

tiBind("fooruby", foo)

puts "ok bound, exiting script"