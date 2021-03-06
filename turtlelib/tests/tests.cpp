/// \file
/// \brief Tests the functions and operators defined in rigid2D.cpp
/// use g++ -Wall -Wextra -g -std=c++17 -o tests tests.cpp rigid2d.cpp to run tests

#include <catch_ros/catch.hpp>
#include <turtlelib/rigid2d.hpp>
#include <turtlelib/diff_drive.hpp>
#include <string>       
#include <iostream>    
#include <sstream> 

using namespace turtlelib;

//////////////////////////////////////////// Inverse Kinematics
TEST_CASE("Inverse kinematics Robot driving forward", "[DiffDrive]") {
    DiffDrive diff = DiffDrive();
    Twist2D twist;
    speed phi_rates;
    twist.x_dot = 1;
    twist.y_dot = 0;
    twist.theta_dot = 0;
    phi_rates = diff.inverse_Kinematics(twist);
    CHECK(phi_rates.left_vel == Approx(30.303030303)); 
    CHECK(phi_rates.right_vel == Approx(30.303030303)); 
}

TEST_CASE("Inverse kinematics Robot driving backward", "[DiffDrive]") {
    DiffDrive diff = DiffDrive();
    Twist2D twist;
    speed phi_rates;
    twist.x_dot = -1;
    twist.y_dot = 0;
    twist.theta_dot = 0;
    phi_rates = diff.inverse_Kinematics(twist);
    CHECK(phi_rates.left_vel == Approx(-30.303030303)); 
    CHECK(phi_rates.right_vel == Approx(-30.303030303)); 
}


TEST_CASE("Inverse kinematics Diff Pure Rotation", "[DiffDrive]") {
    DiffDrive diff = DiffDrive();
    Twist2D twist;
    speed phi_rates;
    twist.x_dot = 0;
    twist.y_dot = 0;
    twist.theta_dot = PI/2;
    phi_rates = diff.inverse_Kinematics(twist);
    CHECK(phi_rates.left_vel == Approx(-3.8079910953)); 
    CHECK(phi_rates.right_vel == Approx(3.8079910953));  
}

TEST_CASE("Inverse kinematics Diff Pure Rotation Inverse", "[DiffDrive]") {
    DiffDrive diff = DiffDrive();
    Twist2D twist;
    speed phi_rates;
    twist.x_dot = 0;
    twist.y_dot = 0;
    twist.theta_dot = -PI/2;
    phi_rates = diff.inverse_Kinematics(twist);
    CHECK(phi_rates.left_vel == Approx(3.8079910953)); 
    CHECK(phi_rates.right_vel == Approx(-3.8079910953)); 
}


TEST_CASE("Inverse kinematics Diff Follow Arc", "[DiffDrive]") {
    DiffDrive diff = DiffDrive();
    Twist2D twist;
    speed phi_rates;
    twist.x_dot = PI/4;
    twist.y_dot = 0;
    twist.theta_dot = PI/2;
    phi_rates = diff.inverse_Kinematics(twist);
    CHECK(phi_rates.left_vel == Approx(19.9919532501)); 
    CHECK(phi_rates.right_vel == Approx(27.6079354406)); 
}


TEST_CASE("Inverse kinematics Impossible to follow test", "[DiffDrive]") {
    DiffDrive diff = DiffDrive();
    Twist2D twist;
    speed phi_rates;
    twist.x_dot = 1;
    twist.y_dot = 1;
    twist.theta_dot = PI/2;
    CHECK_THROWS(phi_rates = diff.inverse_Kinematics(twist));
}

//////////////////////////////////////////// Inverse Kinematics
//////////////////////////////////////////// Forwad Kinematics

TEST_CASE("Forward kinematics Robot driving forward", "[DiffDrive]") {
    config config_,new_config;
    phi_angles phi_,phi_new,phi_old;
    speed phi_dot;
    config_.x = 0;
    config_.y = 0;
    config_.theta = 0;
    phi_.left_angle = 0;
    phi_.right_angle = 0;
    phi_dot.left_vel = 0;
    phi_dot.right_vel = 0;
    DiffDrive diff = DiffDrive(config_,phi_,phi_dot);

    phi_new.left_angle = PI/4;
    phi_new.right_angle = PI/4;

    new_config = diff.forward_Kinematics(phi_new);
    CHECK(new_config.x == Approx(0.033*(PI/4))); 
    CHECK(new_config.y == Approx(0));
    CHECK(new_config.theta == Approx(0)); 
}

TEST_CASE("Forward kinematics Robot driving rotation", "[DiffDrive]") {
    config config_,new_config;
    phi_angles phi_,phi_new,phi_old;
    speed phi_dot;
    config_.x = 0;
    config_.y = 0;
    config_.theta = 0;
    phi_.left_angle = 0;
    phi_.right_angle = 0;
    phi_dot.left_vel = 0;
    phi_dot.right_vel = 0;
    DiffDrive diff = DiffDrive(config_,phi_,phi_dot);

    phi_new.left_angle = PI/4;
    phi_new.right_angle = -PI/4;

    new_config = diff.forward_Kinematics(phi_new);
    CHECK(new_config.x == Approx(0.0)); 
    CHECK(new_config.y == Approx(0));
    CHECK(new_config.theta == Approx(-0.3239767424)); 
}


TEST_CASE("Forward kinematics Robot driving arc", "[DiffDrive]") {
    config config_,new_config;
    phi_angles phi_,phi_new,phi_old;
    speed phi_dot;
    Twist2D twist;
    
    config_.x = 0;
    config_.y = 0;
    config_.theta = 0;
    phi_.left_angle = 0;
    phi_.right_angle = 0;
    phi_dot.left_vel = 0;
    phi_dot.right_vel = 0;
    DiffDrive diff = DiffDrive(config_,phi_,phi_dot);

    phi_new.left_angle = 19.992;
    phi_new.right_angle = 27.6079;
    
    new_config = diff.forward_Kinematics(phi_new);
    CHECK(new_config.x == Approx(0.5)); 
    CHECK(new_config.y == Approx(0.5));
    CHECK(new_config.theta == Approx(PI/2)); 
}

//////////////////////////////////////////// Forwad Kinematics
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TEST_CASE("forward Kinematics", "[DiffDrive]") {
    config config_,new_config;
    phi_angles phi_,phi_new,phi_old;
    speed phi_dot;
    config_.x = 0;
    config_.y = 0;
    config_.theta = 0;
    phi_.left_angle = 0;
    phi_.right_angle = 0;
    phi_dot.left_vel = 0;
    phi_dot.right_vel = 0;
    DiffDrive diff = DiffDrive(config_,phi_,phi_dot);

    phi_new.left_angle = PI/4;
    phi_new.right_angle = PI/4;

    new_config = diff.forward_Kinematics(phi_new);
    CHECK(new_config.x == Approx(0.033*(PI/4))); 
    CHECK(new_config.y == Approx(0));
    CHECK(new_config.theta == Approx(0)); 
}

TEST_CASE("Integrating a Twist", "[rigid2d]") {
    Twist2D twist;
    Transform2D TbbPrime;
    Vector2D trans;
    double rot; 
    SECTION( "Testing only Translational components" ) {
        twist.x_dot = 3.0;
        twist.y_dot = 5.0;
        twist.theta_dot = 0.0;
        TbbPrime = integrate_twist(twist);
        trans = TbbPrime.translation();
        rot = TbbPrime.rotation();
        CHECK( trans.x ==  Approx(3.0));
        CHECK( trans.y ==  Approx(5.0));
        CHECK( rot ==  Approx(0.0));
    }
    
    SECTION( "Testing only Rotational components" ) {
        twist.x_dot = 0.0;
        twist.y_dot = 0.0;
        twist.theta_dot = PI;
        TbbPrime = integrate_twist(twist);
        trans = TbbPrime.translation();
        rot = TbbPrime.rotation();
        CHECK( trans.x ==  Approx(0.0));
        CHECK( trans.y ==  Approx(0.0));
        CHECK( rot ==  Approx(PI));
    }

    SECTION( "Testing Both Rotational and Translational components" ) {
        twist.x_dot = 3.0;
        twist.y_dot = 5.0;
        twist.theta_dot = PI/2;
        TbbPrime = integrate_twist(twist);
        trans = TbbPrime.translation();
        rot = TbbPrime.rotation();
        CHECK( trans.x == Approx(-1.27324));
        CHECK( trans.y ==  Approx(5.09296));
        CHECK( rot ==  Approx(PI/2));
    }

    SECTION( "Testing Both Rotational and Translational components v2" ) {
        twist.x_dot = 1.0;
        twist.y_dot = 2.0;
        twist.theta_dot = PI;
        TbbPrime = integrate_twist(twist);
        trans = TbbPrime.translation();
        rot = TbbPrime.rotation();
        CHECK( trans.x == Approx(-1.27324));
        CHECK( trans.y ==  Approx(0.63662));
        CHECK( rot ==  Approx(PI));
    }

    SECTION( "Testing Both Rotational and Translational components v3" ) {
        twist.x_dot = 2.0;
        twist.y_dot = 4.0;
        twist.theta_dot = PI;
        TbbPrime = integrate_twist(twist);
        trans = TbbPrime.translation();
        rot = TbbPrime.rotation();
        CHECK( trans.x == Approx(-2.54).margin(0.01));
        CHECK( trans.y ==  Approx(1.272).margin(0.01));
        CHECK( rot ==  Approx(PI));
    }

    SECTION( "Testing Both Rotational and Translational components v4" ) {
        twist.x_dot = 1.0;
        twist.y_dot = 1.0;
        twist.theta_dot = PI/2;
        TbbPrime = integrate_twist(twist);
        trans = TbbPrime.translation();
        rot = TbbPrime.rotation();
        CHECK( trans.x == Approx(0.0).margin(0.01));
        CHECK( trans.y ==  Approx(4/PI).margin(0.01));
        CHECK( rot ==  Approx(PI/2));
    }
}


TEST_CASE("Normalizing Angles", "[rigid2d]") {

    CHECK( normalize_angle(PI) ==  Approx(PI));
    CHECK( normalize_angle(-PI) ==  Approx(PI));
    CHECK( normalize_angle(0) ==  Approx(0));
    CHECK( normalize_angle(-PI/4) ==  Approx(-PI/4));
    CHECK( normalize_angle(3*PI/2) ==  Approx(-PI/2));
    CHECK( normalize_angle(-5*PI/2) ==  Approx(-PI/2));
}

TEST_CASE("+= Test", "[transform]") {
    Vector2D vect1;
    Vector2D vect2;
    vect1.x = 1.0;
    vect1.y = 3.0;
    vect2.x = 2.0;
    vect2.y = 4.0;
    vect2 += vect1;
    CHECK( vect2.x == 3.0 );
    CHECK( vect2.y == 7.0 );
}

TEST_CASE("+ Test", "[transform]") {
    Vector2D vect1;
    Vector2D vect2;
    Vector2D vect3;
    vect1.x = 1.0;
    vect1.y = 3.0;
    vect2.x = 2.0;
    vect2.y = 4.0;
    vect3 = vect1 + vect2;
    CHECK( vect3.x == 3.0 );
    CHECK( vect3.y == 7.0 );
}

TEST_CASE("-= Test", "[transform]") {
    Vector2D vect1;
    Vector2D vect2;
    vect1.x = 1.0;
    vect1.y = 2.0;
    vect2.x = 5.0;
    vect2.y = 3.0;
    vect2 -= vect1;
    CHECK( vect2.x == 4.0 );
    CHECK( vect2.y == 1.0 );
}

TEST_CASE("- Test", "[transform]") {
    Vector2D vect1;
    Vector2D vect2;
    Vector2D vect3;
    vect1.x = 1.0;
    vect1.y = 2.0;
    vect2.x = 5.0;
    vect2.y = 3.0;
    vect3 = vect2 - vect1;
    CHECK( vect3.x == 4.0 );
    CHECK( vect3.y == 1.0 );
}

TEST_CASE("*= Test", "[transform]") {
    Vector2D vect;
    vect.x = 1.0;
    vect.y = 2.0;
    vect *= 2;
    CHECK( vect.x == 2.0 );
    CHECK( vect.y == 4.0 );
}

TEST_CASE("* Test", "[transform]") {
    Vector2D vect;
    vect.x = 1.0;
    vect.y = 2.0;
    vect = vect * 2;
    CHECK( vect.x == 2.0 );
    CHECK( vect.y == 4.0 );
}

TEST_CASE("*1 Test", "[transform]") {
    Vector2D vect;
    vect.x = 1.0;
    vect.y = 2.0;
    vect = 2 * vect;
    CHECK( vect.x == 2.0 );
    CHECK( vect.y == 4.0 );
}

TEST_CASE("Test Identity Transform", "[transform]") {
    Transform2D transform = Transform2D();
    Vector2D vec = transform.translation();
    double angle = transform.rotation();
    double x_ph = vec.x;
    double y_ph = vec.y;

    CHECK( angle == 0 );
    CHECK( x_ph == 0 );
    CHECK( y_ph == 0 );
}

TEST_CASE("Test Translational Only Transform", "[transform]") {
    Vector2D vector;
    vector.x = 2.0;
    vector.y = 3.0;
    Transform2D transform = Transform2D(vector);
    Vector2D vec = transform.translation();
    double angle = transform.rotation();
    double x_ph = vec.x;
    double y_ph = vec.y;

    CHECK( angle == 0 );
    CHECK( x_ph == 2.0 );
    CHECK( y_ph == 3.0 );
}

TEST_CASE("Test Rotational Only Transform", "[transform]") {
    double angle_init = 90.0;
    Transform2D transform = Transform2D(angle_init);
    Vector2D vec = transform.translation();
    double angle = transform.rotation();
    double x_ph = vec.x;
    double y_ph = vec.y;

    CHECK( angle == 90.0 );
    CHECK( x_ph == 0.0 );
    CHECK( y_ph == 0.0 );
}

TEST_CASE("Test Both Translational and Rotational Transform", "[transform]") {
    double angle_init = 90.0;
    Vector2D vector;
    vector.x = 2.0;
    vector.y = 3.0;
    Transform2D transform = Transform2D(vector,angle_init);
    Vector2D vec = transform.translation();
    double angle = transform.rotation();
    double x_ph = vec.x;
    double y_ph = vec.y;

    CHECK( angle == 90.0 );
    CHECK( x_ph == 2.0 );
    CHECK( y_ph == 3.0 );
}

TEST_CASE("Test Apply Transform for Vector2D", "[transform]") {
    Vector2D init_vect;
    Vector2D new_vect;
    double angle_init = deg2rad(180.0);
    init_vect.x = 2.0;
    init_vect.y = 4.0;
    Transform2D transform = Transform2D(angle_init);
    new_vect = transform(init_vect);

    CHECK(new_vect.x== Approx(-2.0));
    CHECK(new_vect.y==Approx(-4.0));
}

TEST_CASE("Test Apply Transform for Twist2D", "[transform]") {
    Twist2D init_twist;
    Twist2D new_twist;
    double angle_init = deg2rad(180.0);
    init_twist.theta_dot = 1.0;
    init_twist.x_dot = 2.0;
    init_twist.y_dot = 4.0;
    Transform2D transform = Transform2D(angle_init);
    new_twist = transform(init_twist);
    CHECK(new_twist.theta_dot== Approx(1.0));
    CHECK(new_twist.x_dot== Approx(-2.0));
    CHECK(new_twist.y_dot== Approx(-4.0));
}

TEST_CASE("Test Inverse of a Transform", "[transform]") {
    double angle_init = deg2rad(180.0);
    Vector2D init_vect;
    init_vect.x = 2.0;
    init_vect.y = 4.0;
    Transform2D transform = Transform2D(init_vect,angle_init);
    Transform2D inv_transform = transform.inv();
    Vector2D inv_vect = inv_transform.translation();
    double inv_angle = inv_transform.rotation();

    CHECK(inv_vect.x== Approx(2.0));
    CHECK(inv_vect.y== Approx(4.0));
    CHECK(inv_angle== Approx(-deg2rad(180.0)));
}

TEST_CASE("Test Multiply Equals", "[transform]") {
    Vector2D vect1;
    Vector2D vect2;
    Vector2D translational_comp;
    double angle1 = deg2rad(90.0);
    double angle2 = deg2rad(90.0);
    double rotational_comp;
    vect1.x = 1;
    vect1.y = 2;
    vect2.x = 3;
    vect2.y = 4;
    Transform2D transform1 = Transform2D(vect1,angle1);
    Transform2D transform2 = Transform2D(vect2,angle2);
    transform1 *= transform2;
    translational_comp = transform1.translation();
    rotational_comp = transform1.rotation();

    CHECK(translational_comp.x== Approx(-3.0));
    CHECK(translational_comp.y== Approx(5.0));
    CHECK(rotational_comp== deg2rad(180.0));

}

TEST_CASE("Test Translation and Rotational Functions", "[transform]") {
    Vector2D vect;
    double angle = deg2rad(90.0);
    vect.x = 6.0;
    vect.y = 12.0;
    Transform2D transform = Transform2D(vect,angle);

    SECTION( "Testing Translational components" ) {
        Vector2D vect_comp;
        vect_comp = transform.translation();
        CHECK(vect_comp.x== Approx(6.0));
        CHECK(vect_comp.y== Approx(12.0));
    }

    SECTION( "Testing Rotational components" ) {
        double angle_comp;
        angle_comp = transform.rotation();
        CHECK(angle_comp== deg2rad(90.0));
    }
}

TEST_CASE("Test Input stream", "[transform]") {
    std::stringstream ss;

    SECTION( "Just numbers" ) {
        ss << 90 << ' '<< 0 << ' ' << 1;
        int deg,x,y;
        ss >> deg >>x>>y;
        CHECK(deg == 90);
        CHECK(x == 0);
        CHECK(y == 1);
    }

    SECTION( "Strings added" ) {
        ss.peek();
        ss << "deg: " << 90 << " x: "<< 0 << " y: " << 1;
        int deg,x,y;
        ss >> deg >>x>>y;
        CHECK(deg == 90);
        CHECK(x == 0);
        CHECK(y == 1);
    }
}

TEST_CASE("Test Output stream", "[transform]") {
    std::stringstream ss;
    std::string s("deg: 90 x: 0 y: 1");
    ss << "deg: " << 90 << " x: "<< 0 << " y: " << 1; 
    CHECK(s == ss.str());
}

TEST_CASE("Test Multiply two transforms", "[transform]") {
    Vector2D vect1;
    Vector2D vect2;
    Vector2D translational_comp;
    double angle1 = deg2rad(45.0);
    double angle2 = deg2rad(45.0);
    vect1.x = 4;
    vect1.y = 5;
    vect2.x = 1;
    vect2.y = 2;
    Transform2D transform1 = Transform2D(vect1,angle1);
    Transform2D transform2 = Transform2D(vect2,angle2);
    Transform2D product = transform1*transform2;
    Vector2D prod_vect = product.translation();
    double prod_angle = product.rotation();
    CHECK(prod_vect.x == Approx((4-sqrt(2)/2)));
    CHECK(prod_vect.y == Approx((5+3*sqrt(2)/2)));
    CHECK(prod_angle == (PI/2));
}
