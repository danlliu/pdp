  1 2 3
8 O O O
7      
6      
5     O
4      
-------             lac     five
3 O    
2      
1 O    
8 O O O
7      
6     O
5      
4      
------- 1:          add     negone
3 O    
2 O    
1      
8 O O O
7      
6     O
5     O
4      
-------             sza
3     O
2      
1   O  
8 O O O
7      
6     O
5     O
4      
-------             jmp     1b
3      
2      
1 O    
8 O O O
7      
6     O
5     O
4     O
-------             hlt
3   O O
2     O
1      
8 O O O
7      
6      
5      
4      
------- five:       .fill   5
3 O    
2      
1 O    
8 O O O
7      
6 O O O
5 O O O
4 O O O
------- negone:     .fill   -1
3 O O O
2 O O O
1   O O
