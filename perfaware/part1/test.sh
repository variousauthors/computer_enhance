./build.sh && ./main listing_0037_single_register_mov > out.asm 
nasm out.asm 
diff out listing_0037_single_register_mov

./build.sh && ./main listing_0038_many_register_mov > out.asm 
nasm out.asm 
diff out listing_0038_many_register_mov

./build.sh && ./main listing_0039_more_movs > out.asm 
nasm out.asm 
diff out listing_0039_more_movs

./build.sh && ./main listing_0040_challenge_movs > out.asm 
nasm out.asm 
diff out listing_0040_challenge_movs

./build.sh && ./main listing_0041_add_sub_cmp_jnz > out.asm 
nasm out.asm 
diff out listing_0041_add_sub_cmp_jnz

./build.sh && ./main listing_0043_immediate_movs > out.asm 
nasm out.asm 
diff out listing_0043_immediate_movs

./build.sh && ./main listing_0044_register_movs > out.asm 
nasm out.asm 
diff out listing_0044_register_movs

./build.sh && ./main listing_0045_challenge_register_movs > out.asm 
nasm out.asm 
diff out listing_0045_challenge_register_movs

./build.sh && ./main listing_0046_add_sub_cmp > out.asm 
nasm out.asm 
diff out listing_0046_add_sub_cmp