#include <iostream>
#include <armadillo>
#include<cmath>
#include "nuslam/nuslam.hpp"
#include <turtlelib/diff_drive.hpp>
#include <turtlelib/rigid2d.hpp>

namespace nuslam
{
    // odometry model
    KalmanFilter::KalmanFilter()
    {
        n = 3; //number of obstacles
        predict_state_est=arma::mat(3+2*n,1,arma::fill::zeros);
        qt = arma::mat(3,1,arma::fill::zeros);
        mt = arma::mat(2*n,1,arma::fill::ones);

        Q = arma::mat(3,3,arma::fill::eye);
        // Q(0,0) = 1.1;
        // Q(1,1) = 1.1;
        // Q(2,2) = 1.1;

        R = arma::mat(2,2,arma::fill::eye);
        R(0,0) = 0.001;
        R(1,1) = 0.001;

        current_measure.r_j = 0.0;
        current_measure.phi_j = 0.0;

        predict_cov_est = arma::mat(3+2*n,3+2*n,arma::fill::zeros);
        predict_cov_est.submat(3,3 ,2*n+3-1,2*n+3-1) = 1000000.0*arma::mat(2*n,2*n,arma::fill::eye);
        predict_cov_est.print();


    }

    arma::mat KalmanFilter::calculate_transition(turtlelib::Twist2D twist, double rate)
    {
        double delta_x = twist.x_dot/rate;
        double delta_t = twist.theta_dot/rate;
        double top_term;
        double bot_term;  
        arma::mat A;
        arma::mat I = arma::mat(2*n+3,2*n+3,arma::fill::eye);

        if(turtlelib::almost_equal(delta_t,0.0,0.001))
        {
            top_term = -delta_x*sin(predict_state_est(0,0));
            bot_term = delta_x*cos(predict_state_est(0.0));
        }
        else
        {
            top_term = -(delta_x/delta_t)*cos(turtlelib::normalize_angle(predict_state_est(0,0))) + (delta_x/delta_t)*cos(turtlelib::normalize_angle(predict_state_est(0,0)+delta_t));
            bot_term = -(delta_x/delta_t)*sin(turtlelib::normalize_angle(predict_state_est(0,0))) + (delta_x/delta_t)*sin(turtlelib::normalize_angle(predict_state_est(0,0)+delta_t));
        }

        arma::mat tl;
        tl = {  {0.0,0.0,0.0},
                {top_term,0.0,0.0},
                {bot_term,0.0,0.0}  };

        arma::mat tr(3,2*n,arma::fill::zeros);
        arma::mat br(2*n,2*n,arma::fill::zeros);
        arma::mat bl(2*n,3,arma::fill::zeros);
        arma::mat second_term = arma::mat(2*n+3,2*n+3,arma::fill::zeros);
        second_term(1,0) = top_term;
        second_term(2,0) = bot_term;
        // auto joined_top  = std::move(arma::join_rows( tl, tr ));
        // auto joined_bot  = std::move(arma::join_rows( bl, br ));
        // auto joined_all  = std::move(arma::join_cols( joined_top, joined_bot ));

        A = I + second_term;
        return A;
    }

    arma::mat KalmanFilter::calculate_updated_state(turtlelib::Twist2D twist, double rate)
    {
        double delta_x = twist.x_dot/rate;
        double delta_t = twist.theta_dot/rate;
        arma::mat T_wp_prime =  arma::mat(3,1,arma::fill::zeros);
        arma::mat w_t =  arma::mat(3,1,arma::fill::zeros);

        if(turtlelib::almost_equal(delta_t,0.0,0.001))
        {
            T_wp_prime(0,0) = 0.0;
            T_wp_prime(1,0) = delta_x*cos(turtlelib::normalize_angle(predict_state_est(0.0)));
            T_wp_prime(2,0) = delta_x*sin(turtlelib::normalize_angle(predict_state_est(0.0)));
        }
        else
        {
            T_wp_prime(0,0) = delta_t;
            T_wp_prime(1,0) = -(delta_x/delta_t)*sin((predict_state_est(0,0))) + (delta_x/delta_t)*sin((predict_state_est(0,0)+delta_t));
            T_wp_prime(2,0) = (delta_x/delta_t)*cos((predict_state_est(0,0))) - (delta_x/delta_t)*cos((predict_state_est(0,0)+delta_t));
        }

        predict_state_est(0,0) = turtlelib::normalize_angle(predict_state_est(0,0)+ T_wp_prime(0,0) + w_t(0,0)); 
        predict_state_est(1,0) = predict_state_est(1,0)+ T_wp_prime(1,0) + w_t(1,0); 
        predict_state_est(2,0) = predict_state_est(2,0)+ T_wp_prime(2,0) + w_t(2,0); 
        return predict_state_est;

    }

    arma::mat KalmanFilter::calculate_h(int j)  //also known as z
    {
        arma::mat h =  arma::mat(2,1,arma::fill::zeros);
        double rj, phij, temp_angle;
        rj = sqrt(pow(predict_state_est((2*j)+1,0)-predict_state_est(1,0),2)+pow(predict_state_est((2*j)+2,0)-predict_state_est(2,0),2));
        temp_angle = atan2(predict_state_est((2*j)+2,0)-predict_state_est(2,0),predict_state_est((2*j)+1,0)-predict_state_est(1,0))-predict_state_est(0,0);
        phij = turtlelib::normalize_angle(temp_angle);
        h(0,0) = rj;
        h(1,0) = phij;

        return h;
    }

    arma::mat KalmanFilter::calculate_H(int j)
    {
        double delta_x = predict_state_est((2*j)+1,0)- predict_state_est(1,0); 
        double delta_y = predict_state_est((2*j)+2,0) - predict_state_est(2,0);
        double d = delta_x*delta_x+delta_y*delta_y;
        arma::mat complete_h = arma::mat(2,3+2*n,arma::fill::zeros);

        complete_h(0,0) = 0.0;
        complete_h(0,1) = -delta_x/sqrt(d);
        complete_h(0,2) = -delta_y/sqrt(d);
        complete_h(1,0) = -1.0;
        complete_h(1,1) = delta_y/d;
        complete_h(1,2) = -delta_x/d;
        
        complete_h(0,3+2*(j-1)) = delta_x/sqrt(d);
        complete_h(0,4+2*(j-1)) = delta_y/sqrt(d);
        complete_h(1,3+2*(j-1)) = -delta_y/d;
        complete_h(1,4+2*(j-1)) = delta_x/d;

        return complete_h;
    }


    arma::mat KalmanFilter::calculate_Q_est()
    {
        arma::mat Q_bar(3+2*n,3+2*n,arma::fill::zeros);
        Q_bar.submat(0,0, 2,2) = Q;
        return Q_bar;

    }

    arma::mat KalmanFilter::predict(turtlelib::Twist2D twist,double rate)
    {
        arma::mat Q_bar(3+2*n,3+2*n,arma::fill::zeros);
        Q_bar.submat(0,0, 2,2) = Q;
        arma::mat A = calculate_transition(twist,rate);
        predict_state_est = calculate_updated_state(twist,rate);
        // predict_cov_est
        // A.print();
        // predict_cov_est.print();
        // Q_bar.print();
        predict_cov_est = A * predict_cov_est * A.t() + Q_bar;
        // predict_cov_est.print();
        return predict_state_est;
    }

    arma::mat KalmanFilter::update(int num, arma::mat prev_z)
    {
        arma::mat z(2,1,arma::fill::zeros);
        arma::mat delta_z(2,1,arma::fill::zeros);

        for (int i=0;i<num;i++)
        {
            double z_radius = prev_z(2*i,0);
            double z_angle = turtlelib::normalize_angle(prev_z((2*i)+1,0));
            z(0,0) = z_radius;
            z(1,0) = z_angle;
            arma::mat I = arma::mat(2*n+3,2*n+3,arma::fill::eye);
            //first calculate the theoretical measurement 
            arma::mat zt = calculate_h(i+1);
            arma::mat H = calculate_H(i+1);
            // // kalman gain
            arma::mat Ki = predict_cov_est*H.t()*inv(H*predict_cov_est*H.t()+R);//I removed R
            H.print();
            delta_z = (z-zt);
            delta_z(1,0) = turtlelib::normalize_angle(delta_z(1,0));
            predict_state_est = predict_state_est + Ki*(delta_z);
            predict_state_est(0,0) = turtlelib::normalize_angle(predict_state_est(0,0));
            predict_cov_est = (I-Ki*H)*predict_cov_est;
            // predict_state_est.print();S

        }
        // predict_state_est.print();
        return predict_state_est;

    }

    arma::mat KalmanFilter::Landmark_Initialization(int robot_id, turtlelib::Vector2D coords)
    {
        predict_state_est(3+2*robot_id,0) = predict_state_est(1,0)+coords.x*cos(turtlelib::normalize_angle(predict_state_est(0,0)+coords.y));
        predict_state_est(3+2*robot_id+1,0) = predict_state_est(2,0)+coords.x*sin(turtlelib::normalize_angle(predict_state_est(0,0)+coords.y));
    }
    
}
// //another way to get H
// arma::mat KalmanFilter::calculate_H(int j)
//     {
//         double delta_x = mt(j,0) - predict_state_est(1,0); 
//         double delta_y = mt(j,1) - predict_state_est(2,0);
//         double d = delta_x*delta_x+delta_y*delta_y;
//         arma::mat first = arma::mat(2,3,arma::fill::zeros);
//         arma::mat second = arma::mat(2,2*(j-1),arma::fill::zeros);
//         arma::mat third = arma::mat(2,3,arma::fill::zeros);
//         arma::mat fourth = arma::mat(2,(2*n-2*j),arma::fill::zeros);

//         first(0,0) = 0.0;
//         first(0,1) = -delta_x/sqrt(d);
//         first(0,2) = -delta_y/sqrt(d);
//         first(1,0) = -1.0;
//         first(1,1) = delta_y/d;
//         first(1,2) = -delta_x/d;
        
//         third(0,0) = delta_x/sqrt(d);
//         third(0,1) = delta_y/sqrt(d);
//         third(1,0) = -delta_y/d;
//         third(1,1) = delta_x/d;

//         arma::mat H = std::move(arma::join_rows( first, second, third, fourth));
//         return H;
//     }



// arma::mat KalmanFilter::calculate_H(int j)
    // {
    //     double delta_x;
    //     double delta_y;
    //     double d;
    //     std::vector<arma::mat> h;
    //     h.resize(n);
    //     arma::mat complete_h(2,3+2*n,arma::fill::zeros);
        

    //     for (int i=0;i<j;i++)
    //     {
    //         //create vector of n rows and 2 columns
    //         //resize it to n
    //         //
    //         h[i] = arma::mat(2,3+2*n,arma::fill::zeros);

    //         delta_x = mt(i,0) - predict_state_est(1,0); 
    //         delta_y = mt(i,1) - predict_state_est(2,0);
    //         d = delta_x*delta_x+delta_y*delta_y;

    //         h[i](0,0) = 0.0;
    //         h[i](0,1) = -delta_x/sqrt(d);
    //         h[i](0,2) = -delta_y/sqrt(d);
    //         h[i](1,0) = -1.0;
    //         h[i](1,1) = delta_y/d;
    //         h[i](1,2) = -delta_x/d;
            
    //         h[i](0,3+2*i) = delta_x/sqrt(d);
    //         h[i](0,4+2*i) = delta_y/sqrt(d);
    //         h[i](1,3+2*i) = -delta_y/d;
    //         h[i](1,4+2*i) = delta_x/d;
    //     }

    //     for (int k =0;k<j;k++)
    //     {
    //         if (k == 0)
    //         {
    //             complete_h = h[k];
    //         }
    //         else
    //         {
    //             complete_h = std::move(arma::join_cols( complete_h, h[k] ));
    //         }
    //     }
    //     return complete_h;

    // }