
-- add target
add_target("demo")

    -- add the dependent target
    add_deps("tbox")

    -- make as a binary
    set_kind("binary")

    -- add defines
    add_defines("__tb_prefix__=\"demo\"")

    -- set the object files directory
    set_objectdir("$(buildir)/.objs")

    -- add links directory
    add_linkdirs("$(buildir)/tbox.pkg/lib/$(plat)/$(arch)")

    -- add includes directory
    add_includedirs("$(buildir)/tbox.pkg/inc")
    add_includedirs("$(buildir)/tbox.pkg/inc/$(plat)/$(arch)")

    -- add packages
    add_options("zlib", "mysql", "sqlite3", "openssl", "polarssl")

    -- add links
    add_links("tbox")

    -- add the source files
    add_files("src/demo/*.c") 
    add_files("src/demo/libc/*.c") 
    add_files("src/demo/libm/integer.c") 
    add_files("src/demo/math/random.c") 
    add_files("src/demo/utils/url.c") 
    add_files("src/demo/utils/crc.c") 
    add_files("src/demo/utils/md5.c") 
    add_files("src/demo/utils/sha.c") 
    add_files("src/demo/utils/bits.c") 
    add_files("src/demo/utils/dump.c") 
    add_files("src/demo/utils/base*.c") 
    add_files("src/demo/other/test.c") 
    add_files("src/demo/other/flv.c") 
    add_files("src/demo/other/libflv.c") 
    add_files("src/demo/string/*.c") 
    add_files("src/demo/memory/**.c") 
    add_files("src/demo/network/**.c") 
    add_files("src/demo/platform/*.c") 
    add_files("src/demo/container/*.c") 
    add_files("src/demo/algorithm/*.c") 
    add_files("src/demo/stream/stream.c") 
    add_files("src/demo/stream/stream/*.c") 

    -- add the source files for the float type
    if options("float") then
        add_files("src/demo/math/fixed.c")
        add_files("src/demo/libm/float.c")
        add_files("src/demo/libm/double.c")
    end

    -- add the source files for the xml module
    if options("xml") then
        add_files("src/demo/xml/*.c")
    end

    -- add the source files for the asio module
    if options("asio") then
        add_files("src/demo/asio/*.c")
        add_files("src/demo/stream/async_stream.c")
        add_files("src/demo/stream/transfer_pool.c")
        add_files("src/demo/stream/async_transfer.c")
        add_files("src/demo/stream/async_stream/*.c")
    end

    -- add the source files for the object module
    if options("object") then
        add_files("src/demo/utils/option.c")
        add_files("src/demo/object/jcat.c")
        add_files("src/demo/object/json.c")
        add_files("src/demo/object/bin.c")
        add_files("src/demo/object/xml.c")
        add_files("src/demo/object/xplist.c")
        add_files("src/demo/object/bplist.c")
        add_files("src/demo/object/dump.c")
    end

    -- add the source files for the charset module
    if options("charset") then add_files("src/demo/other/charset.c") end

    -- add the source files for the database module
    if options("database") then add_files("src/demo/database/sql.c") end
    