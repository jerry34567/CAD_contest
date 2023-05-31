module top ( x0 , x1 , y0 , y1 , z , u0 , u1 , w0 , w1 ) ;
    input x0 , x1 , y0 , y1 , z ;
    output u0 , u1 , w0 , w1 ;
    wire n1 , n2 , n3 ;
    nand ( n1 , x1 , y0 ) ;
    nand ( n2 , x0 , y1 ) ;
    nand ( n3 , y0 , z ) ;
    and ( u0 , y0 , x0 ) ;
    xor ( u1 , n1 , n2 ) ;
    xnor ( w0 , y1 , n3 ) ;
    xor ( w1 , y0 , z ) ;
endmodule
