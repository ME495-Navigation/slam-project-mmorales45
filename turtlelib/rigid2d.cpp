#include "rigid2d.hpp"
#include <iostream>


std::ostream & turtlelib::operator<<(std::ostream & os, const Vector2D & v) 
{
    return os << "["<<v.x << " "<< v.y<<"]"<<"\n";
}

std::istream & turtlelib::operator>>(std::istream & is, Vector2D & v) 
{
    char c1 = is.peek();
    
    if (c1 == '['){ //remove '[' if this char appears
        is.get();
    }
    is >> v.x >> v.y;
    return is;
}

turtlelib::Transform2D::Transform2D()
{
    translational_component.x = 0;
    translational_component.y = 0;
    angular_displacement = 0;

} 
turtlelib::Transform2D::Transform2D(Vector2D trans) 
{
    translational_component.x = trans.x;
    translational_component.y = trans.y;
    angular_displacement = 0;
}

turtlelib::Transform2D::Transform2D(double radians)
{
    translational_component.x = 0;
    translational_component.y = 0;
    angular_displacement = radians;
}

turtlelib::Transform2D::Transform2D(Vector2D trans, double radians)
{
    translational_component.x = trans.x;
    translational_component.y = trans.y;
    angular_displacement = radians;
}

turtlelib::Vector2D turtlelib::Transform2D::operator()(Vector2D v) const{
    turtlelib::Vector2D transformed_vector;
    transformed_vector.x = v.x*cos(angular_displacement) - (v.y*sin(angular_displacement)) + translational_component.x;
    transformed_vector.y = -v.x*sin(angular_displacement) + (v.y*cos(angular_displacement)) + translational_component.y;

    return transformed_vector;
}

turtlelib::Transform2D turtlelib::Transform2D::inv() const
{
    double inverse_theta;
    turtlelib::Vector2D inverse_translation;
    turtlelib::Transform2D inverse_transform;

    inverse_theta = -angular_displacement;
    inverse_translation.x = (-translational_component.x *cos(angular_displacement)) - (translational_component.y * sin(angular_displacement));
    inverse_translation.x = (-translational_component.y *cos(angular_displacement)) + (translational_component.x * sin(angular_displacement));

    inverse_transform = Transform2D(inverse_translation, inverse_theta);
    return inverse_transform;
}

turtlelib::Transform2D & turtlelib::Transform2D::operator*=(const Transform2D & rhs){
    translational_component.x = (rhs.translational_component.x)*cos(angular_displacement) - (rhs.translational_component.y)*sin(angular_displacement) + translational_component.x;
    translational_component.y = (rhs.translational_component.x)*sin(angular_displacement) + (rhs.translational_component.x)*cos(angular_displacement) + translational_component.y;
    angular_displacement = angular_displacement+rhs.angular_displacement;
    return *this;
}

turtlelib::Vector2D turtlelib::Transform2D::translation() const
{
    return translational_component;
}

double turtlelib::Transform2D::rotation() const
{
    return angular_displacement;
}

// std::istream & turtlelib::operator>>(std::istream & is, Transform2D & tf){
//     // char str[4];

//     // if (str == "get"){
//     //     is.get(str,5);
//     is >> tf;
//     // }
//     return is;
// }

// std::ostream & turtlelib::operator<<(std::ostream & os, const Transform2D & tf){
//     return os << tf;
// }