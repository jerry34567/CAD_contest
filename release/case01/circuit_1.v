module top ( a0 , a1 , b0 , b1 , c , h0 , h1 , m0 , m1 ) ;
    input a0 , a1 , b1 , b0 , c ;
    output h0 , h1 , m0 , m1 ;
    wire n1 , n2 , n3 , n4 , n5 ;
    not ( n1 , b1 ) ;
    not ( n2 , a0 ) ;
    and ( h0 , a0 , b0 ) ;
    nand ( n3 , a1 , b0 ) ;
    or ( n4 , n1 , n2 ) ;
    nand ( n5 , b0 , c ) ;
    xor ( h1 , n3 , n4 ) ;
    xnor ( m0 , b1 , n5 ) ;
    xor ( m1 , b0 , c ) ;
endmodule

