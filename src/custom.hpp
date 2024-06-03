// this is a custom class to get around min and max function issues
#ifndef CUSTOM_H
#define CUSTOM_H

class Custom{
private:
    /* data */
public:
template <typename T>
T myMin(const T& a, const T& b) {
    return (a < b) ? a : b;
}

template <typename T>
T myMax(const T& a, const T& b) {
    return (a > b) ? a : b;
}

};
template<typename T>
T myAVG(const T& a, const T& b){
    return (a + b)/2;
};


#endif // CUSTOM_H