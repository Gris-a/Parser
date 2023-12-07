KEYWORD(ADD, "+",
{
    return lvalue + rvalue;
},
{
    ans = __ADD(__DIF(LEFT), __DIF(RIGHT));
    break;
},
{
    if(VAR_T(LEFT) && VAR_T(RIGHT) && VARCMP(LEFT->data.var, RIGHT->data.var))
    {
        copy = __MUL(__NUM(2), __VAR(LEFT->data.var));
    }
    else if(NUM_T(LEFT) && DBLCMP(LEFT->data.num, 0))
    {
        copy = __CPY(RIGHT);
    }
    else if(NUM_T(RIGHT) && DBLCMP(RIGHT->data.num, 0))
    {
        copy = __CPY(LEFT);
    }
    break;
})

KEYWORD(SUB, "-",
{
    return lvalue - rvalue;
},
{
    ans = __SUB(__DIF(LEFT), __DIF(RIGHT));
    break;
},
{
    if(KWORD_T(RIGHT) && (RIGHT->data.kword == SUB))
    {
        copy = __ADD(__SUB(__CPY(LEFT), __CPY(RIGHT->left)), __CPY(RIGHT->right));
    }
    else if(KWORD_T(RIGHT) && (RIGHT->data.kword == ADD))
    {
        copy = __SUB(__SUB(__CPY(LEFT), __CPY(RIGHT->left)), __CPY(RIGHT->right));
    }
    else if(VAR_T(LEFT) && VAR_T(RIGHT) && VARCMP(LEFT->data.var, RIGHT->data.var))
    {
        copy = __NUM(0);
    }
    else if(NUM_T(RIGHT) && DBLCMP(RIGHT->data.num, 0))
    {
        copy = __CPY(LEFT);
    }
    else if(NUM_T(LEFT) && DBLCMP(LEFT->data.num, 0))
    {
        copy = __MUL(__NUM(-1), __CPY(RIGHT));
    }
    break;
})

KEYWORD(MUL, "*",
{
    return lvalue * rvalue;
},
{
    ans = __ADD(__MUL(__CPY(RIGHT), __DIF(LEFT)),
                __MUL(__DIF(RIGHT), __CPY(LEFT)));
    break;
},
{
    if((NUM_T(LEFT)  && DBLCMP(LEFT->data.num , 0)) ||
       (NUM_T(RIGHT) && DBLCMP(RIGHT->data.num, 0)))
    {
        copy = __NUM(0);
    }
    else if(NUM_T(LEFT) && DBLCMP(LEFT->data.num, 1))
    {
        copy = __CPY(RIGHT);
    }
    else if(NUM_T(RIGHT) && DBLCMP(RIGHT->data.num, 1))
    {
        copy = __CPY(LEFT);
    }
    else if(VAR_T(LEFT) && VAR_T(RIGHT) && VARCMP(LEFT->data.var, RIGHT->data.var))
    {
        copy = __POW(__CPY(LEFT), __NUM(2));
    }
    break;
})

KEYWORD(DIV, "/",
{
    return lvalue / rvalue;
},
{
    ans = __DIV(__SUB(__MUL(__CPY(RIGHT), __DIF(LEFT)),
                      __MUL(__DIF(RIGHT), __CPY(LEFT))),
                __POW(__CPY(RIGHT), __NUM(2)));
    break;
},
{
    if(KWORD_T(RIGHT) && (RIGHT->data.kword == DIV))
    {
        copy = __MUL(__DIV(__CPY(LEFT), __CPY(RIGHT->left)), __CPY(RIGHT->right));
    }
    else if(VAR_T(LEFT) && VAR_T(RIGHT) && VARCMP(LEFT->data.var, RIGHT->data.var))
    {
        copy = __NUM(1);
    }
    else if(NUM_T(LEFT) && DBLCMP(LEFT->data.num, 0))
    {
        copy = __NUM(0);
    }
    else if(NUM_T(RIGHT) && DBLCMP(RIGHT->data.num, 1))
    {
        copy = __CPY(LEFT);
    }
    break;
})

KEYWORD(POW, "^",//TODO pow to log() power
{
    return pow(lvalue, rvalue);
},
{
    if(NUM_T(RIGHT))
    {
        ans = __MUL(__POW(__CPY(LEFT), __NUM(RIGHT->data.num - 1)),
                    __MUL(__NUM(RIGHT->data.num), __DIF(LEFT)));
    }
    else
    {
        ans = __MUL(__CPY(node),
                    __ADD(__DIV(__MUL(__CPY(RIGHT), __DIF(LEFT)),
                                __CPY(LEFT)),
                          __MUL(__DIF(RIGHT), __LN(__CPY(LEFT)))));
    }
    break;
},
{
    if(KWORD_T(LEFT) && (LEFT->data.kword == POW))
    {
        copy = __POW(__CPY(LEFT->left), __MUL(__CPY(LEFT->right), __CPY(RIGHT)));
    }
    else if((NUM_T(LEFT)  && DBLCMP(LEFT->data.num , 1)) ||
            (NUM_T(RIGHT) && DBLCMP(RIGHT->data.num, 0)))
    {
        copy = __NUM(1);
    }
    else if(NUM_T(LEFT) && DBLCMP(LEFT->data.num, 0))
    {
        copy = __NUM(0);
    }
    else if(NUM_T(RIGHT) && DBLCMP(RIGHT->data.num, 1))
    {
        copy = __CPY(LEFT);
    }
    break;
})