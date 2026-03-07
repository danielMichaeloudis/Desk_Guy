use std::ffi::c_char;

#[no_mangle]
extern "C" fn rust_main() -> i32 {
    println!("aaa");
    unsafe {
        lcd_init();
        sd_init();
        list_fs();
        check_exists(c"TEST_I~1.BMP".as_ptr());
        draw_bmp_file(c"TEST_I~1.BMP".as_ptr())
    };
    32
}

extern "C" {
    pub fn draw_bmp_file(name: *const c_char);
    pub fn lcd_init();
    pub fn sd_init() -> i32;
    pub fn list_fs();
    pub fn check_exists(name: *const c_char) -> bool;
}
