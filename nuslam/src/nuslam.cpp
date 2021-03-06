#include <iostream>
#include <armadillo>
#include<cmath>
#include "nuslam/nuslam.hpp"
#include <turtlelib/diff_drive.hpp>
#include <turtlelib/rigid2d.hpp>
#include <ros/console.h>


namespace nuslam
{
    // odometry model

    KalmanFilter::KalmanFilter()              
    {
    }
    
    KalmanFilter::KalmanFilter(int num_obs,double Q_scale,double R_scale) : n(num_obs),predict_state_est(arma::mat(3+2*n,1,arma::fill::zeros)),
                                            Q(Q_scale * arma::mat(3,3,arma::fill::eye)),
                                            R(R_scale * arma::mat(2,2,arma::fill::eye)),
                                            predict_cov_est(arma::mat(3+2*n,3+2*n,arma::fill::zeros))               
    {
        predict_cov_est.submat(3,3 ,2*n+3-1,2*n+3-1) = 100000000.0*arma::mat(2*n,2*n,arma::fill::eye);
    }

    arma::mat KalmanFilter::calculate_transition(turtlelib::Twist2D twist)
    {
        double delta_x = twist.x_dot/1.0;
        double delta_t = twist.theta_dot/1.0;
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

        arma::mat second_term = arma::mat(2*n+3,2*n+3,arma::fill::zeros);
        second_term(1,0) = top_term;
        second_term(2,0) = bot_term;

        A = I + second_term;
        return A;
    }

    arma::mat KalmanFilter::calculate_updated_state(turtlelib::Twist2D twist)
    {
        double delta_x = twist.x_dot/1.0;
        double delta_t = twist.theta_dot/1.0;
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

        predict_state_est(0,0) = turtlelib::normalize_angle(predict_state_est(0,0)+ T_wp_prime(0,0)); 
        predict_state_est(1,0) = predict_state_est(1,0)+ T_wp_prime(1,0); 
        predict_state_est(2,0) = predict_state_est(2,0)+ T_wp_prime(2,0); 
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

    arma::mat KalmanFilter::calculate_H(int j, arma::mat newState)
    {
        double delta_x = newState((2*j)+1,0)- newState(1,0); 
        double delta_y = newState((2*j)+2,0) - newState(2,0);
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


    arma::mat KalmanFilter::predict(turtlelib::Twist2D twist)
    {
        arma::mat Q_bar(3+2*n,3+2*n,arma::fill::zeros);
        Q_bar.submat(0,0, 2,2) = Q;
        arma::mat A = calculate_transition(twist);
        predict_state_est = calculate_updated_state(twist);
        predict_cov_est = A * predict_cov_est * A.t() + Q_bar;
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

    void KalmanFilter::Landmark_Initialization(int robot_id, arma::mat coords)
    {
        predict_state_est(3+2*robot_id,0) = predict_state_est(1,0)+coords(0,0)*cos(turtlelib::normalize_angle(predict_state_est(0,0)+coords(1,0)));
        predict_state_est(3+2*robot_id+1,0) = predict_state_est(2,0)+coords(0,0)*sin(turtlelib::normalize_angle(predict_state_est(0,0)+coords(1,0)));
    }

    void KalmanFilter::Landmark_Initialization(int robot_id, arma::mat coords, turtlelib::config config)
    {
        double a = config.theta;
        double b = config.x;
        double c= config.y;

        predict_state_est(3+2*robot_id,0) = b+coords(0,0)*cos(turtlelib::normalize_angle(a+coords(1,0)));
        predict_state_est(3+2*robot_id+1,0) = c+coords(0,0)*sin(turtlelib::normalize_angle(a+coords(1,0)));
    }

    int KalmanFilter::DataAssociation(arma::mat z){
        double d_star = 1000.0;
        double ma_THRESH = 0.1;
        arma::mat maha_distance;
        double maha_distance_val;
        //If n is 0, skip loop and increment N.
        //Create new state based on current measurement for comparison
                
        for (int i=0;i<N;i++)
        {
            //Loop through every value in N to determine if the malahanobis dis is smaller than thresh
            
            

            double delta_x = predict_state_est((2*i)+3,0)- predict_state_est(1,0); 
            double delta_y = predict_state_est((2*i)+4,0) - predict_state_est(2,0);
            double d = delta_x*delta_x+delta_y*delta_y;
            arma::mat H_k = arma::mat(2,3+2*n,arma::fill::zeros);

            H_k(0,0) = 0.0;
            H_k(0,1) = -delta_x/sqrt(d);
            H_k(0,2) = -delta_y/sqrt(d);
            H_k(1,0) = -1.0;
            H_k(1,1) = delta_y/d;
            H_k(1,2) = -delta_x/d;
            
            H_k(0,3+2*(i)) = delta_x/sqrt(d);
            H_k(0,4+2*(i)) = delta_y/sqrt(d);
            H_k(1,3+2*(i)) = -delta_y/d;
            H_k(1,4+2*(i)) = delta_x/d;

            arma::mat covariance = H_k * predict_cov_est * H_k.t() + R;

            // arma::mat z_k = calculate_h(i+1);
            arma::mat z_k =  arma::mat(2,1,arma::fill::zeros);
            double rj, phij, temp_angle;
            rj = sqrt(pow(predict_state_est((2*i)+3,0)-predict_state_est(1,0),2)+pow(predict_state_est((2*i)+4,0)-predict_state_est(2,0),2));
            temp_angle = atan2(predict_state_est((2*i)+4,0)-predict_state_est(2,0),predict_state_est((2*i)+3,0)-predict_state_est(1,0))-predict_state_est(0,0);
            phij = turtlelib::normalize_angle(temp_angle);
            z_k(0,0) = rj;
            z_k(1,0) = phij;
            z_k(1,0) = turtlelib::normalize_angle(z_k(1,0));

            arma::mat delta_z;
            delta_z = (z-z_k);
            delta_z(1,0) = turtlelib::normalize_angle(delta_z(1,0));
            maha_distance = delta_z.t() * inv(covariance) * delta_z;
            maha_distance_val = maha_distance(0,0);
            ROS_INFO_STREAM(maha_distance_val);

            if (maha_distance_val < ma_THRESH)
            {
                N++;
                return N;
            }
            if (maha_distance_val<d_star)
            {
                d_star = maha_distance_val;
            }
        }
        // ROS_INFO_STREAM(N);
        if (N == 0)
        {
            N++;
            return N;
        }
        
        N++;
        return N;
    }

    int KalmanFilter::DataAssociation_V2(arma::mat z){
        arma::mat maha_distance;
        

        if (N == 0)
        {
            N++;
            return 0;
        }

        std::vector<double> mala_distance;  
        for (auto i : mala_distance)
        {
            double delta_x = predict_state_est((2*i)+3,0)- predict_state_est(1,0); 
            double delta_y = predict_state_est((2*i)+4,0) - predict_state_est(2,0);
            double d = delta_x*delta_x+delta_y*delta_y;
            arma::mat H_k = arma::mat(2,3+2*n,arma::fill::zeros);

            H_k(0,0) = 0.0;
            H_k(0,1) = -delta_x/sqrt(d);
            H_k(0,2) = -delta_y/sqrt(d);
            H_k(1,0) = -1.0;
            H_k(1,1) = delta_y/d;
            H_k(1,2) = -delta_x/d;
            
            H_k(0,3+2*(i)) = delta_x/sqrt(d);
            H_k(0,4+2*(i)) = delta_y/sqrt(d);
            H_k(1,3+2*(i)) = -delta_y/d;
            H_k(1,4+2*(i)) = delta_x/d;

            arma::mat covariance = H_k * predict_cov_est * H_k.t() + R;

            // arma::mat z_k = calculate_h(i+1);
            arma::mat z_k =  arma::mat(2,1,arma::fill::zeros);
            double rj, phij, temp_angle;
            rj = sqrt(pow(predict_state_est((2*i)+3,0)-predict_state_est(1,0),2)+pow(predict_state_est((2*i)+4,0)-predict_state_est(2,0),2));
            temp_angle = atan2(predict_state_est((2*i)+4,0)-predict_state_est(2,0),predict_state_est((2*i)+3,0)-predict_state_est(1,0))-predict_state_est(0,0);
            phij = turtlelib::normalize_angle(temp_angle);
            z_k(0,0) = rj;
            z_k(1,0) = phij;
            z_k(1,0) = turtlelib::normalize_angle(z_k(1,0));

            arma::mat delta_z;
            delta_z = (z-z_k);
            delta_z(1,0) = turtlelib::normalize_angle(delta_z(1,0));
            maha_distance = delta_z.t() * inv(covariance) * delta_z;
            double maha_distance_val;
            maha_distance_val = maha_distance(0,0);
        }
        return N;
    }

    void KalmanFilter::UpdateLandmarks(std::vector<turtlelib::Vector2D> known_list)
    {
        known_Landmarks = known_list;
    }

    bool KalmanFilter::CheckLandmarks(std::vector<turtlelib::Vector2D> land_list,turtlelib::Vector2D current_land){
        
        for (int i=0;i<int(land_list.size());i++)
        {
            turtlelib::Vector2D temp_land = land_list.at(i);

            if ((turtlelib::almost_equal(current_land.x,temp_land.x,0.5)) && (turtlelib::almost_equal(current_land.y,temp_land.y,0.5)))
            {
                
                return false;
            }
        }
        ROS_INFO_STREAM("ADDING NEW LANDMARK");
        ROS_INFO_STREAM(current_land);
        return true;
    }


    void KalmanFilter::ResetN()
    {
        N = 0;
    }

    arma::mat KalmanFilter::getMAP()
    {
        arma::mat mapSTATE = predict_state_est.submat(2,0,2+2*n,0);
        return mapSTATE;
    }
    
}
