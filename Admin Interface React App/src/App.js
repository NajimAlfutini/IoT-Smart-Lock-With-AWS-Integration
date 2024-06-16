// App.js
import { BrowserRouter } from "react-router-dom";
import AuthenticatedApp from './AuthenticatedApp';

// This is the main App component. It sets up the router and renders the AuthenticatedApp component.
function App() {
  return (
    <BrowserRouter>
      <AuthenticatedApp />
    </BrowserRouter>
  );
}

export default App;
