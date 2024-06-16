import { Routes, Route, NavLink } from "react-router-dom";
import Login from "./Login";
import AdminConsole  from "./AdminConsole";
import PrivateRoute from "./routes/PrivateRoute";
import PublicRoute from "./routes/PublicRoute";
import React, { useState, useEffect } from "react";
import {
  getAdmin,
  getToken,
  setAdminSession,
  resetAdminSession,
} from "./service/AuthService";
import axios from "axios";

const apiUrl = process.env.REACT_APP_API_URL;
const apiKey = process.env.REACT_APP_API_KEY;

const adminApiUrl = `${apiUrl}/verify`;

export default function AuthenticatedApp() {
  const [isAuthenicating, setAuthenicating] = useState(true);
  const [isLoggedIn, setIsLoggedIn] = useState(false); // new state variable for tracking login status
  


  useEffect(() => {
    const token = getToken();
    if (!token) {
      setAuthenicating(false);
      return;
    }
    const requestConfig = {
      headers: {
        "x-api-key": apiKey,
      },
    };
    const requestBody = {
      admin: getAdmin(),
      token: token,
    };
    axios
    .post(adminApiUrl, requestBody, requestConfig)
    .then((response) => {
      if (response.data.verified) {
        setAdminSession(response.data.admin, response.data.token);
        setIsLoggedIn(true); // set isLoggedIn to true when login is successful
      } else {
        resetAdminSession();
        setIsLoggedIn(false); // set isLoggedIn to false when login fails
      }
      setAuthenicating(false);
    })
    .catch((error) => {
      resetAdminSession();
      setIsLoggedIn(false); // set isLoggedIn to false when there's an error
      setTimeout(() => {
      }, 1500);
      setAuthenicating(false);
    });
}, []);

  const token = getToken();
  console.log(isAuthenicating);
  if (isAuthenicating && token) {
    return <div className="content">Authenicating....</div>;
  }

  return (
    <div className="app-container">
      <div className="header-container">
        {!isLoggedIn && ( // render the "Login" link only if isLoggedIn is false
          <NavLink className="nav-link" activeClassName="active" to="/">
            Login
          </NavLink>
        )}
        <NavLink className="nav-link" activeClassName="active" to="/admin_console">
          Admin Console
        </NavLink>
      </div>
      <div className="content-container" data-page="content">
        <Routes>
          <Route
            exact="true"
            path="/"
            element={
              <PublicRoute>
                <Login />
              </PublicRoute>
            }
          />
          <Route
            exact="true"
            path="/admin_console"
            element={
              <PrivateRoute>
                <AdminConsole />
              </PrivateRoute>
            }
          />
        </Routes>
      </div>
    </div>
  );
}
