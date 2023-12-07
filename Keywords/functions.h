KEYWORD(SIN, "sin",
{
    return sin(rvalue);
},
{
    ans = __MUL(__COS(__CPY(RIGHT)), __DIF(RIGHT));
    break;
},
{
    break;
})

KEYWORD(COS, "cos",
{
    return cos(rvalue);
},
{
    ans = __MUL(__MUL(__NUM(-1), __SIN(__CPY(RIGHT))),
                __DIF(RIGHT));
    break;
},
{
    break;
})

KEYWORD(LN, "ln",
{
    return log(rvalue);
},
{
    ans = __MUL(__POW(__CPY(RIGHT), __NUM(-1)),
                __DIF(RIGHT));
    break;
},
{
    break;
})