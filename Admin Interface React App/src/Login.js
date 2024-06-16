import axios from "axios";
import React, { useState } from "react";
import CryptoJS from "crypto-js";
import { setAdminSession } from "./service/AuthService";
import { useNavigate } from 'react-router-dom';

const apiUrl = process.env.REACT_APP_API_URL;
const apiKey = process.env.REACT_APP_API_KEY;

const adminApiUrl = `${apiUrl}/login`;

// This component handles the login form and authentication.
const Login = () => {
  const navigate = useNavigate();
  const [username, setUsername] = useState("");
  const [password, setPassword] = useState("");
  const [StatusMessage, setStatusMessage] = useState(null);
  const [showPopup, setShowPopup] = useState(false);

  const submitHandler = (event) => {
    event.preventDefault();
    if (username.trim() === "" || password.trim() === "") {
      setStatusMessage("Both AdminName and password are required");
      return;
    }
    setStatusMessage(null);
    const requesteConfig = {
      headers: {
        "x-api-key": apiKey,
      },
    };
    const hashedPassword = CryptoJS.MD5(password).toString();
    const requesteBody = {
      admin: username,
      password: hashedPassword,
    };
    axios
      .post(adminApiUrl, requesteBody, requesteConfig)
      .then((response) => {
        try {
          setAdminSession(response.data.admin, response.data.token);
          navigate('/Admin_console');
          window.location.reload();
        } catch (error) {
          setStatusMessage('Error setting admin session:', error);
        }
        setShowPopup(true)

      })
      .catch((error) => {
        if (error.response) {
          if (error.response.status === 402 || error.response.status === 404) {
            setStatusMessage(error.response.data);
          } else {
            setStatusMessage("An error occurred. Please try again later.");
          }
        } else {
          setStatusMessage("A network error occurred. Please check your connection and try again. OR check your connection");
        }
        setShowPopup(true); // Corrected here
      });

      setTimeout(() => {
        setStatusMessage(null);
        setShowPopup(false);
    }, 3000);
  };
  
  return (
    <div className="loginPage">
      <div className="loginContainer">
      {showPopup === true || StatusMessage !== null ? (
  <div className="status-popup">
    {StatusMessage}
  </div>
) : null}

        <h1 className="smartlockTitle">Smartlock</h1>
        <form className="loginForm" onSubmit={submitHandler}>
          <h5> Login </h5>
          AdminName:{" "}
          <input
            className="usernameInput"
            type="text"
            value={username}
            onChange={(event) => setUsername(event.target.value)}
          />
          password :{" "}
          <input
            className="passwordInput"
            autoComplete="on"
            type="password"
            value={password}
            onChange={(event) => setPassword(event.target.value)}
          />
          <input className="submitButton" type="submit" value="Login" />
        </form>
        {StatusMessage && <p className="message">{StatusMessage}</p>}
      </div>
    </div>
  );
};

export default Login;
