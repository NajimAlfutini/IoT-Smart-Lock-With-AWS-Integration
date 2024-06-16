import React, { useState, useEffect } from "react";
import axios from "axios";
import { getAdmin, resetAdminSession } from "./service/AuthService";
import { useNavigate } from "react-router-dom";
import "@fortawesome/fontawesome-free/css/all.css";
import SearchBar from "./SearchBar"; // Assuming SearchBar is in the same directory

const apiUrl = process.env.REACT_APP_API_URL;
const apiKey = process.env.REACT_APP_API_KEY;

const openLockApiUrl = `${apiUrl}/openlock`;
const LockLogsApiUrl = `${apiUrl}/updates`;
const getUsersApiUrl = `${apiUrl}/users`; //will be create soon
const For_UserApiUrl = `${apiUrl}/user`; //will be create soon

const ForUserAddFingr = `${For_UserApiUrl}/addFingerID`; //will be create soon
const ForUserAddRFID = `${For_UserApiUrl}/addRFID`; //will be create soon
const ForUserAddPass = `${For_UserApiUrl}/addPassword`; //will be create soon
const ForUserAddTOKEN = `${For_UserApiUrl}/addToken`; //will be create soon

const AdminConsole = (props) => {
  const navigate = useNavigate();
  const admin = getAdmin();
  const name = admin ? admin.name : "";

  const [statusMessage, setStatusMessage] = useState(null);
  const [logs, setLogs] = useState([]);
  const [filteredLogs, setFilteredLogs] = useState([]);
  const [showLogs, setShowLogs] = useState(false); // State for showing logs
  const [showUsers, setShowUsers] = useState(false); // State for showing users
  const [users, setUsers] = useState([]);
  const [user, setUser] = useState({
    ID: "",
    firstName: "",
    lastName: "",
    email: "",
  });
  const [fingerID, setFingerID] = useState("");
  const [tokenState, setTokenState] = useState("");
  const [exp, setExp] = useState("");
  const [showState, setShowState] = useState(false); // State for showing state
  const [exp2, setExp2] = useState("");
  const [showPopup, setShowPopup] = useState(false); // State for showing the pop-up
  const [currentStage, setCurrentStage] = useState(1);
  const [undo, setUndo] = useState(false);

  const requestConfig = {
    headers: {
      "x-api-key": apiKey,
    },
  };

  const clearBody = () => {
    setCurrentStage(0);
    setShowLogs(false);
    setShowUsers(false);
    setShowState(false);
  };

  const resetUser = () => {
    setUser({ ID: "", firstName: "", lastName: "", email: "" });
  };

  const openLockHandler = async () => {
    clearBody();
    const requestBody = {
      admin: admin,
      name: name,
    };
    try {
      const response = await axios.post(
        openLockApiUrl,
        requestBody,
        requestConfig
      );
      if (response.status === 200) {
        setStatusMessage(`${response.data.message} "${response.data.name}"`);
      } else {
        setStatusMessage("An error occurred. Please try again later.");
      }
      setShowPopup(true);
    } catch (error) {
      if (
        error.response &&
        error.response.status >= 500 &&
        error.response.status < 600
      ) {
        setStatusMessage("A server error occurred. Please try again later.");
        setShowPopup(true);
      }
    }
    setTimeout(() => {
      setStatusMessage(null);
      setShowPopup(false);
    }, 3000);
  };

  const getLogsHandler = () => {
    clearBody();
    axios
      .get(LockLogsApiUrl, requestConfig) // Include the access token in the headers
      .then((response) => {
        if (response.status === 200) {
          let sortedLogs = response.data.updates.sort(
            (a, b) => new Date(b.timestamp) - new Date(a.timestamp)
          );
          setLogs(sortedLogs);
          setFilteredLogs(sortedLogs); // Initialize filteredLogs with all logs
          setStatusMessage("logs Table");
          setShowLogs(true); // Show the logs after fetching
        } else {
          setStatusMessage(
            "An error occurred while fetching logs. Please try again later."
          );
        }
        setShowPopup(true);
      })
      .catch((error) => {
        if (
          error.response &&
          error.response.status >= 500 &&
          error.response.status < 600
        ) {
          setStatusMessage("A server error occurred. Please try again later.");
          setShowPopup(true);
        }
      });
    setTimeout(() => {
      setStatusMessage(null);
      setShowPopup(false);
    }, 3000);
  };

  const getUsersHandler = () => {
    clearBody();
    axios
      .get(getUsersApiUrl, requestConfig) // Include the access token in the headers
      .then((response) => {
        if (response.status === 200) {
          setUsers(response.data.users);
          setShowUsers(true); // Show the users data after fetching
          setShowLogs(false); // Hide the logs when showing users
          setStatusMessage("Users Table");
        } else {
          setStatusMessage(
            "An error occurred while fetching users. Please try again later."
          );
        }
        setShowPopup(true);
      })
      .catch((error) => {
        if (
          error.response &&
          error.response.status >= 500 &&
          error.response.status < 600
        ) {
          setStatusMessage("A server error occurred. Please try again later.");
          setShowPopup(true);
        }
      });
    setTimeout(() => {
      setStatusMessage(null);
      setShowPopup(false);
    }, 3000);
  };

  const deleteUserHandler = (id) => {
    if (window.confirm("Are you sure?")) {
      clearBody();
      axios
        .delete(For_UserApiUrl, {
          data: { ID: id },
          headers: { "x-api-key": apiKey },
        }) // Include the access token in the headers
        .then((response) => {
          if (!undo) {
            setStatusMessage("User deleted successfully.");
            setShowPopup(true);
          }
          getUsersHandler(); // Refresh the users list after deleting a user
        })
        .catch((error) => {
          if (
            error.response &&
            error.response.status >= 500 &&
            error.response.status < 600
          ) {
            setStatusMessage(
              "A server error occurred. Please try again later."
            );
            setShowPopup(true);
          } else {
            setStatusMessage(error.response.data);
            setShowPopup(true);
          }
        });
      setTimeout(() => {
        setStatusMessage(null);
        setShowPopup(false);
      }, 3000);
      setUndo(false);
    } else return;
  };

  const logoutHandler = () => {
    if (window.confirm("Are you sure you want to log out?")) {
      resetAdminSession();
      navigate("/");
      window.location.reload();
    }
  };

  const addUserHandler = (event) => {
    event.preventDefault();
    if (user.ID && user.firstName && user.lastName && user.email) {
      const requestBody = {
        ID: user.ID,
        "First Name": user.firstName,
        "Last Name": user.lastName,
        Email: user.email,
      };
      axios
        .post(For_UserApiUrl, requestBody, requestConfig)
        .then((response) => {
          setStatusMessage(response.data.Message);
          setShowPopup(true);
          setUndo(true);
          setCurrentStage(2); // Move to the next stage only if the request is successful
        })
        .catch((error) => {
          if (
            error.response &&
            error.response.status >= 500 &&
            error.response.status < 600
          ) {
            setStatusMessage(
              "A server error occurred. Please try again later."
            );
            setShowPopup(true);
          } else {
            setCurrentStage(1);
            setStatusMessage(error.response.data.Message);
            setShowPopup(true);
          }
        });
      setTimeout(() => {
        setStatusMessage(null);
        setShowPopup(false);
      }, 3000);
    } else {
      setStatusMessage("Please fill in all fields.");
      setCurrentStage(1);
      setShowPopup(true);
      setTimeout(() => {
        setStatusMessage(null);
        setShowPopup(false);
      }, 3000);
    }
  };

  const addFingerIDHandler = (event) => {
    event.preventDefault();
    const data = { ID: user.ID, FingerPrintID: fingerID };
    axios
      .post(ForUserAddFingr, data, requestConfig)
      .then((response) => {
        setCurrentStage(3);
        setStatusMessage(response.data);
        setShowPopup(true);
      })
      .catch((error) => {
        if (
          error.response &&
          error.response.status >= 500 &&
          error.response.status < 600
        ) {
          setStatusMessage("A server error occurred. Please try again later.");
          setShowPopup(true);
        } else {
          setCurrentStage(2);
          setStatusMessage(error.response.data);
          setShowPopup(true);
        }
      });
    setTimeout(() => {
      setStatusMessage(null);
      setShowPopup(false);
    }, 3000);
  };

  const addRFIDHandler = () => {
    const data = { ID: user.ID };
    axios
      .post(ForUserAddRFID, data, requestConfig)
      .then((response) => {
        setCurrentStage(4);
        setStatusMessage(response.data);
        setShowPopup(true);
      })
      .catch((error) => {
        if (
          error.response &&
          error.response.status >= 500 &&
          error.response.status < 600
        ) {
          setStatusMessage("A server error occurred. Please try again later.");
          setShowPopup(true);
        } else {
          setCurrentStage(3);
          setStatusMessage(error.response.data);
          setShowPopup(true);
        }
      });
    setTimeout(() => {
      setStatusMessage(null);
      setShowPopup(false);
    }, 3000);
  };

  const addPasswordHandler = () => {
    const data = { ID: user.ID };
    axios
      .post(ForUserAddPass, data, requestConfig)
      .then((response) => {
        setCurrentStage(5);
        setStatusMessage(response.data);
        setShowPopup(true);
      })
      .catch((error) => {
        if (
          error.response &&
          error.response.status >= 500 &&
          error.response.status < 600
        ) {
          setStatusMessage("A server error occurred. Please try again later.");
          setShowPopup(true);
        } else {
          setCurrentStage(4);
          setStatusMessage(error.response.data);
          setShowPopup(true);
        }
      });
    setTimeout(() => {
      setStatusMessage(null);
      setShowPopup(false);
    }, 3000);
  };

  const addTokenHandler = (event) => {
    event.preventDefault();
    if (tokenState === "") {
      setStatusMessage("Please select a timestamp option.");
      setShowPopup(true);
      setTimeout(() => {
        setStatusMessage(null);
        setShowPopup(false);
      }, 3000);
      return;
    }
    let expFormatted = exp
      ? tokenState === "3"
        ? exp + ":00"
        : new Date(exp).toLocaleDateString("en-GB") +
          " " +
          new Date(exp).toTimeString().substr(0, 8)
      : "-";
    let exp2Formatted = exp2
      ? tokenState === "3"
        ? exp2 + ":00"
        : new Date(exp2).toLocaleDateString("en-GB") +
          " " +
          new Date(exp2).toTimeString().substr(0, 8)
      : "-";

    if (new Date(exp2) < new Date(exp)) {
      setStatusMessage(
        "Please ensure the ending time is later than the starting time."
      );
      setShowPopup(true);
      setTimeout(() => {
        setStatusMessage(null);
        setShowPopup(false);
      }, 3000);
      return;
    }

    const data = { id: user.ID, state: tokenState, exp: expFormatted };
    if (tokenState === "2") {
      data.exp += " - " + exp2Formatted;
    }
    if (tokenState === "3") {
      data.exp += " " + exp2Formatted;
    }
    if (tokenState < 1 || tokenState > 3) {
      data.exp = "-";
    }

    axios
      .post(ForUserAddTOKEN, data, requestConfig)
      .then((response) => {
        getUsersHandler();
        setUndo(false);
        setStatusMessage(response.data);
        setShowPopup(true);
      })
      .catch((error) => {
        if (
          error.response &&
          error.response.status >= 500 &&
          error.response.status < 600
        ) {
          setStatusMessage("A server error occurred. Please try again later.");
          setShowPopup(true);
        } else {
          setCurrentStage(5);
          setStatusMessage(error.response.data);
          setShowPopup(true);
        }
      });
    
  };

  const showStagesHandler = () => {
    clearBody();
    setCurrentStage(1);
    setShowState(true); // Show the stages
  };

  const resetAddUser = (flag) => {
    if (!undo){
        return false;
    } 
    if (user.ID || user.ID !== "" || flag === true) {
      if (window.confirm("Are you sure!!")) {
        axios
          .delete(For_UserApiUrl, {
            data: { ID: user.ID },
            headers: { "x-api-key": apiKey },
          }) // Include the access token in the headers
          .then((response) => {
            console.log(response.data);
            setUndo(false);
            resetUser();
            clearBody();
          })
          .catch((error) => {
            setStatusMessage(
              "A server error occurred. Please try again later."
            );
            setShowPopup(true);
          });
          setTimeout(() => {
            setStatusMessage(null);
            setShowPopup(false);
          }, 3000);
        }else{
            return true;
        }
    }
  };

  // Add useEffect hook to start the timer when the component mounts
  useEffect(() => {
    if (undo) {
      const onBeforeUnload = (ev) => {
          axios
            .delete(For_UserApiUrl, {
              data: { ID: user.ID },
              headers: { "x-api-key": apiKey },
            }) // Include the access token in the headers
            .then((response) => {
              console.log(response.data);
              setUndo(false);
              resetUser();
              clearBody();
            })
            .catch((error) => {
              setStatusMessage(
                "A server error occurred. Please try again later."
              );
              setShowPopup(true);
            });
            setTimeout(() => {
              setStatusMessage(null);
              setShowPopup(false);
            }, 3000);
            ev.returnValue = "seccess";
      };

      window.addEventListener("beforeunload", onBeforeUnload);

      return () => {
        window.removeEventListener("beforeunload", onBeforeUnload);
      };
    }
  });

  return (
    <div className="admin-console">
      <h1 className="admin-greeting">Hello {name} !</h1>
      <div className="buttons-container">
        <button
          className="open-lock-button"
          onClick={() => {
            if (!resetAddUser()) openLockHandler();
          }}
        >
          Open Lock
        </button>
        <button
          className="get-logs-button"
          onClick={() => {
            if (!resetAddUser()) getLogsHandler();
          }}
        >
          View Logs
        </button>
        <button
          className="get-users-button"
          onClick={() => {
            if (!resetAddUser()) getUsersHandler();
          }}
        >
          Manage Users
        </button>
        <button className="add-user-button" onClick={showStagesHandler}>
          Add New User
        </button>
        <button
          className="logout-button"
          onClick={() => {
            if (!resetAddUser()) logoutHandler();
          }}
        >
          Logout
        </button>
        <button
          className="hide-body-button"
          onClick={() => {
            if (!resetAddUser()) clearBody();
          }}
        >
          Hide body
        </button>
      </div>
      {showPopup === true || statusMessage !== null ? (
        <div className="status-popup">{statusMessage}</div>
      ) : null}

      {showLogs && logs.length > 0 && (
        <div>
          <SearchBar logs={logs} setFilteredLogs={setFilteredLogs} />
          {filteredLogs && filteredLogs.length > 0 && (
            <table className="logs-table">
              <thead>
                <tr>
                  <th>Timestamp</th>
                  <th>ID</th>
                  <th>Lock State</th>
                  <th>Name</th>
                </tr>
              </thead>
              <tbody>
                {filteredLogs.map((log, index) => (
                  <tr key={index}>
                    <td>{log.timestamp}</td>
                    <td>{log.ID}</td>
                    <td>{log.LockState}</td>
                    <td>{log.Name}</td>
                  </tr>
                ))}
              </tbody>
            </table>
          )}
        </div>
      )}
      {showUsers && users.length > 0 && (
        <table className="users-table">
          <thead>
            <tr>
              <th>ID</th>
              <th>First Name</th>
              <th>Last Name</th>
              <th>Email</th>
              <th>FingerPrint</th>
              <th>RFID</th>
              <th>Access Token</th>
              <th>Action</th>
            </tr>
          </thead>
          <tbody>
            {users.map((user, index) => (
              <tr key={index}>
                <td>{user.ID}</td>
                <td>{user["First Name"]}</td>
                <td>{user["Last Name"]}</td>
                <td>{user.Email}</td>
                <td>{user.FingerPrint}</td>
                <td>{user.RFID}</td>
                <td>{user.Token_Access}</td>
                <td>
                  <button
                    className="delete-button"
                    onClick={() => deleteUserHandler(user.ID)}
                  >
                    <i className="fa fa-trash" aria-hidden="true"></i>
                  </button>
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      )}
      {showState && (
        <div className="stages-container">
          <div className="progress-bar">
            <div
              className="progress-bar-fill"
              style={{ width: `${((currentStage - 1) / 5) * 100}%` }}
            ></div>
          </div>
          {currentStage > 2 && (
            <button
              className="button-back"
              onClick={() => setCurrentStage(currentStage - 1)}
            >
              Previous Stage
            </button>
          )}
          {currentStage > 1 && (
            <button className="button-undo" onClick={() => resetAddUser()}>
              Cancel Registration
            </button>
          )}

          {currentStage === 1 && (
            <div className="stage stage-add-user">
              <h2>Add User</h2>
              <form onSubmit={addUserHandler}>
                <input
                  type="text"
                  className="input-id"
                  placeholder="ID"
                  onChange={(e) => setUser({ ...user, ID: e.target.value })}
                  required
                />
                <input
                  type="text"
                  className="input-first-name"
                  placeholder="First Name"
                  onChange={(e) =>
                    setUser({ ...user, firstName: e.target.value })
                  }
                  required
                />
                <input
                  type="text"
                  className="input-last-name"
                  placeholder="Last Name"
                  onChange={(e) =>
                    setUser({ ...user, lastName: e.target.value })
                  }
                  required
                />
                <input
                  type="text"
                  className="input-email"
                  placeholder="Email"
                  onChange={(e) => setUser({ ...user, email: e.target.value })}
                  required
                />
                <button type="submit" className="button-submit">
                  Add User
                </button>
              </form>
            </div>
          )}
          {currentStage === 2 && (
            <div className="stage stage-add-finger-id">
              <p>User ID: {user.ID}</p>
              <h2>Add Finger ID</h2>
              <form onSubmit={addFingerIDHandler}>
                <input
                  type="text"
                  className="input-finger-id"
                  placeholder="Finger ID"
                  onChange={(e) => setFingerID(e.target.value)}
                  required
                />
                <button type="submit" className="button-submit">
                  Add Finger ID
                </button>
              </form>
            </div>
          )}
          {currentStage === 3 && (
            <div className="stage stage-add-rfid">
              <p>User ID: {user.ID}</p>
              <h2>Add RFID</h2>
              <button onClick={addRFIDHandler} className="button-submit">
                Add RFID
              </button>
            </div>
          )}
          {currentStage === 4 && (
            <div className="stage stage-add-password">
              <p>User ID: {user.ID}</p>
              <h2>Add Password</h2>
              <button onClick={addPasswordHandler} className="button-submit">
                Add Password
              </button>
            </div>
          )}
          {currentStage === 5 && (
            <div className="stage stage-add-token">
              <p>User ID: {user.ID}</p>
              <h2>Add Token</h2>
              <form onSubmit={addTokenHandler}>
                <select
                  className="select-token-state"
                  onChange={(e) => setTokenState(e.target.value)}
                  required
                >
                  <option value="">Select a timestamp option</option>
                  <option value="1">To this date</option>
                  <option value="2">From this date to this date</option>
                  <option value="3">Daily between these times</option>
                  <option value="4">Always valid</option>
                </select>
                {tokenState === "1" && (
                  <input
                    type="datetime-local"
                    className="input-expiration-time"
                    onChange={(e) => setExp(e.target.value)}
                    required
                  />
                )}
                {tokenState === "2" && (
                  <>
                    <input
                      type="datetime-local"
                      className="input-expiration-time"
                      onChange={(e) => setExp(e.target.value)}
                      required
                    />
                    <input
                      type="datetime-local"
                      className="input-expiration-time-2"
                      onChange={(e) => setExp2(e.target.value)}
                      required
                    />
                  </>
                )}
                {tokenState === "3" && (
                  <>
                    <input
                      type="time"
                      className="input-expiration-time"
                      onChange={(e) => setExp(e.target.value)}
                      required
                    />
                    <input
                      type="time"
                      className="input-expiration-time-2"
                      onChange={(e) => setExp2(e.target.value)}
                      required
                    />
                  </>
                )}
                <button type="submit" className="button-submit">
                  Add Token
                </button>
              </form>
            </div>
          )}
        </div>
      )}
    </div>
  );
};

export default AdminConsole;
