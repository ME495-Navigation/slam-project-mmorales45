#ifndef NUSLAM_INCLUDE_GUARD_HPP
#define NUSLAM_INCLUDE_GUARD_HPP

/// \file
/// \brief Library for an Extended Kalman Filter

#include <iostream>
#include <armadillo>
#include <iosfwd> 
#include <turtlelib/diff_drive.hpp>
#include <turtlelib/rigid2d.hpp>


namespace nuslam
{

    class KalmanFilter
    {
    public:

        /// \brief Create an object that makes matrices wit correct dimensions.
        KalmanFilter();

        /// \brief Create an object that makes matrices wit correct dimensions.
        KalmanFilter(int num_obs,double Q_scale,double R_scale);
        
        /// \brief Update the qt, or x, y, and theta of the robot given a twist.
        /// \param twist- The change in x,y,and theta for one timestep.
        /// \return predict_state_est- the updated state variable with the new x,y,and theta.
        arma::mat calculate_updated_state(turtlelib::Twist2D twist);

        /// \brief Calculate the transition matrix A.
        /// \param twist- The change in x,y,and theta for one timestep.
        /// \return A- Derivative of g with respect to state and twist.
        arma::mat calculate_transition(turtlelib::Twist2D twist);

        /// \brief Calculate the estimated range and phi of an obstacle.
        /// \param j- The obstacle number.
        /// \return h- The range and phi of the j index obstacle.
        arma::mat calculate_h(int j);

        /// \brief Calculate the derivative of the h matrix.
        /// \param j- The obstacle number.
        /// \return H- A 2 by (3+2n) derivative matrix with respect to the state.
        arma::mat calculate_H(int j);

        arma::mat calculate_H(int j, arma::mat newState);
        /// \brief Update the state estimate, and covariance.
        /// \param twist- The change in x,y,and theta for one timestep.
        /// \return predict_state_est- the updated state variable with the new x,y,and theta.
        arma::mat predict(turtlelib::Twist2D twist);

        /// \brief Compute the posterior state and covariance matrix by loooping through every obstacle.
        /// \param num- The number of obstacles.
        /// \param z- A matrix of size 2n by 1 that contains the range and phi for every obstacle.
        /// \return predict_state_est- the updated state variable updates values for each obstacle and config.
        arma::mat update(int num, arma::mat z);

        /// \brief Add an obstacle into the class given the range and phi.
        /// \param robot_id- The specific id of an obstacle.
        /// \param coords- The range and phi of the obstacle.
        void Landmark_Initialization(int robot_id, arma::mat coords);

        void Landmark_Initialization(int robot_id, arma::mat coords, turtlelib::config config);
        
        /// \brief Associate data of unknown landmarks.
        /// \param z- The current Measurements.
        int DataAssociation(arma::mat z);
        
        /// \brief Associate data of unknown landmarks in a different manner.
        /// \param z- The current Measurements.
        int DataAssociation_V2(arma::mat z);

        /// \brief Set the number of known landmarks to 0.
        void ResetN();
        
        /// \brief Set the private variable of known landmarks to the input from node.
        /// \param known_list- The current list of known landmark positions.
        void UpdateLandmarks(std::vector<turtlelib::Vector2D> known_list);

        /// \brief Set the private variable of known landmarks to the input from node.
        /// \param land_list- The current list of known landmark positions.
        /// \param current_land- The current landmark.
        bool CheckLandmarks(std::vector<turtlelib::Vector2D> land_list,turtlelib::Vector2D current_land);

        /// \brief Get the map values of the state vector.
        arma::mat getMAP();

    private:

        int n {};
        arma::mat predict_state_est {};    
        arma::mat Q {};
        arma::mat R {};
        arma::mat predict_cov_est {};
        int N {};
        std::vector<turtlelib::Vector2D> known_Landmarks;
    };
}

#endif