
require 'facets'
require 'facets/enumerable/combos'

def for_template_parameters_upto size
  Array.new(size).map_with_index{ |not_important, i|
    ([(["T"]*4).zip(
    ['', 'const', 'volatile', 'const volatile'],
    ['&']*4)]*(i+1)).
      map_with_index{ |args, arg_i|
        args.map{ |arg|
          # e.g., T0 const& a
          "#{arg.first}#{arg_i} #{arg.tail.join} #{(arg_i+10).to_s(36)}"
        }
      }.combos.map{ |arg| arg.kind_of?(Array) ? arg.join(', ') : arg }
  }.flatten.each{ |args_list| yield args_list }
end

def template_parameters args_list
  size = args_list.count(',') + 1
  '<' + (['class T']*size).map_with_index{ |t, index|
    "#{t}#{index}"
  }.join(', ') + '>'
end

def forward_parameters args_list
  args_list
end

def arguments args_list
  size = args_list.count(',') + 1
  Array.new(size).map_with_index{ |not_important, i|
    (i+10).to_s 36
  }.join(', ')
end