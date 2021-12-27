#ifndef __TOKEN_H__
#define __TOKEN_H__

//The lexer returns tokens [0-255] if it's an unknoen character 
//otherwise it returns one of these for known things 

enum Token{
    //End of file
    tok_eof = -1,

    //Commands 
    tok_def=-2,
    tok_extern=-3,

    //Primary
    tok_identifier = -4,
    tok_number = -5,

};

#endif