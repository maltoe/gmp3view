#ifndef __BASE64_H__
#define __BASE64_H__

string base64_encode(unsigned char const* bytes_to_encode, unsigned int len);
string base64_decode(string const& s);

#endif /* __BASE64_H__ */
