void cprintf(char *fmt, ...);

void fun(int a) 
{ 
    cprintf("Value of a is %d\n", a); 
} 
  
int main() 
{ 
    void (*fun_ptr)(int) = &fun; 
    (*fun_ptr)(10); 
    return(0); 
} 
