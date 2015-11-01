// -*- c++ -*-

#ifndef JSON_H
#define JSON_H

#define BUFLEN 150
// A brain-dead minimal json parser
// - assumes input is correctly formed and no whitespace
// - assumes only one dict containing key/value pairs
// - values must be single value or a list of values
class MyJson {
public:
    // Construct from a character string
    MyJson( char* json );

    // Expect to open a dict - ie. expect a "{'
    bool opendict();

    // Return true if we have parsed the end of the dict
    bool enddict();

    // Return true if we have encountered an error
    bool error();

    // Return the name of the next key - NULL if a keyname is not found next
    char* getkey();

    // Return the string for the next value - NULL is a value is not found
    char* getvalue();

    // Return the enxt value as double - 0.0 if error
    // Need to test separately if an error occured with error().
    double fgetvalue();
private:
    bool  m_endarray;			// Flag - we have parsed end of an array
    bool  m_enddict;			// Flag - we have parsed end of a dict
    bool  m_error;			// Flag - we have encountered an error
    char* m_json;			// Save the json string to this buffer
    char* m_s;				// Pointer to next char to parse
};

class MyJsonSend {
public:
    // Construct from a character string
    MyJsonSend();

    void update( const char* key, double x1 );

    void update( const char* key, double x1, double x2 );

    void update( const char* key, double x1, double x2, double x3 );

    bool send();

private:
    void firsttime();
    void add( const char* s );
    void addi( int i );
    void addui( unsigned long i );
    void addd( double d );

private:
    bool    m_firsttime;
    char    m_json[BUFLEN];
    char*   m_s;
    char*   m_end;
};
    
#endif //JSON_H

// Local Variables:
// c-basic-offset: 4
// End: 
