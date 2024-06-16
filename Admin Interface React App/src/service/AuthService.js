// This module exports functions for managing the admin session.
module.exports = {
  // Retrieves the admin from the session storage.
  getAdmin: function () {
    const admin = sessionStorage.getItem("admin");
    if (admin === "undefined" || !admin) {
      return null;
    } else {
      return JSON.parse(admin);
    }
  },
  // Retrieves the token from the session storage.
  getToken: function () {
    return sessionStorage.getItem("token");
  },
  // Sets the admin and token in the session storage.
  setAdminSession: function (admin, token) {
    sessionStorage.setItem("admin", JSON.stringify(admin));
    sessionStorage.setItem("token", token);
  },
  // Removes the admin and token from the session storage.
  resetAdminSession: function () {
    sessionStorage.removeItem("admin");
    sessionStorage.removeItem("token");
  },
};
