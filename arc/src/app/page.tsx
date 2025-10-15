'use client'

import React, { useState, useEffect } from 'react';

// --- Configuration ---
// IMPORTANT: Replace this placeholder with the actual IP address of your ESP32
const INITIAL_ESP32_IP = '192.168.1.100'; 
const NOMINAL_VOLTAGE = 5.0; // Assume 5.0V for Power calculation on the client side

// --- Lucide Icons (Using simple SVG placeholders for a single-file implementation) ---
// Bolt: Power/Voltage
const Bolt = (props) => <svg {...props} xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round"><path d="M13 2H3l8 11H3l10 9V11h8L13 2z"/></svg>;
// Zap: Current/Lightning
const Zap = (props) => <svg {...props} xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round"><polygon points="13 2 3 14 12 14 11 22 21 10 12 10 13 2"/></svg>;
// CheckCircle/Alert: Status
const CheckCircle = (props) => <svg {...props} xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round"><path d="M22 11.08V12a10 10 0 1 1-5.93-9.14"/><polyline points="22 4 12 14 9 11"/></svg>;

// *** FIX: Changed the invalid closing tag </circleAlert> to the correct self-closing />
const CircleAlert = (props) => <svg {...props} xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round"><circle cx="12" cy="12" r="10"/><line x1="12" x2="12" y1="8" y2="12"/><line x1="12" x2="12.01" y1="16" y2="16"/></svg>;

// Gauge: Power
const Gauge = (props) => <svg {...props} xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round"><path d="M12 12A8 8 0 0 0 4 12M12 12A8 8 0 0 1 20 12M12 12V6M16 12l2 2M8 12l-2 2M12 18V22"/></svg>;

// Helper Component for Data Cards (Styled using shadcn/card conventions)
const CurrentCard = ({ title, value, unit, icon: Icon, colorClass }) => (
  <div className="flex flex-col justify-between rounded-xl border border-gray-200 dark:border-gray-800 bg-white dark:bg-gray-950 p-6 shadow-lg transition-all hover:shadow-xl hover:scale-[1.01] min-w-[250px]">
    <div className="flex items-center justify-between">
      <h3 className="text-sm font-medium text-gray-500 dark:text-gray-400">{title}</h3>
      <Icon className={`w-6 h-6 ${colorClass}`} />
    </div>
    <div className="mt-4">
      <p className="text-4xl font-bold tracking-tight text-gray-900 dark:text-gray-50">
        {value.toFixed(3)}
      </p>
      <p className="text-sm text-gray-500 dark:text-gray-400 mt-1">{unit}</p>
    </div>
  </div>
);

// Helper Component for Device Status
const DeviceStatus = ({ isConnected, ipAddress }) => {
  const statusText = isConnected ? 'Online' : 'Disconnected';
  const statusColor = isConnected ? 'bg-green-500' : 'bg-red-500';
  const statusIcon = isConnected ? CheckCircle : CircleAlert;
  const iconColor = isConnected ? 'text-green-500' : 'text-red-500';

  return (
    <div className="flex flex-col space-y-2 p-3 rounded-xl bg-gray-100 dark:bg-gray-800 border border-gray-300 dark:border-gray-700 shadow-sm">
      <div className="flex items-center space-x-2">
          <div className={`w-3 h-3 rounded-full ${statusColor} animate-pulse`}></div>
          <p className="text-sm font-medium text-gray-700 dark:text-gray-300">
            Device Status: <span className={`font-semibold ${iconColor}`}>{statusText}</span>
          </p>
          {statusIcon({className: `w-5 h-5 ${iconColor}`})}
      </div>
       <p className="text-xs text-gray-500 dark:text-gray-400 font-mono">
            Target IP: <span className="font-semibold">{ipAddress}</span>
      </p>
    </div>
  );
};


// Main component 
const App = () => {
  const [esp32Ip, setEsp32Ip] = useState(INITIAL_ESP32_IP);
  const [liveCurrent, setLiveCurrent] = useState(0.00);
  const [power, setPower] = useState(0.00);
  const [isConnected, setIsConnected] = useState(false);
  const [logMessages, setLogMessages] = useState([]);
  
  // Debounced state for input to prevent excessive API calls while typing
  const [debouncedIp, setDebouncedIp] = useState(INITIAL_ESP32_IP);

  // Effect for handling IP input changes (Debouncing for better performance)
  useEffect(() => {
    const handler = setTimeout(() => {
      setDebouncedIp(esp32Ip);
    }, 500); // Wait 500ms after user stops typing
    
    return () => clearTimeout(handler);
  }, [esp32Ip]);


  // Effect for fetching real-time data from ESP32
  useEffect(() => {
    if (!debouncedIp || debouncedIp === 'YOUR_ESP32_IP_ADDRESS') {
        setIsConnected(false);
        setLiveCurrent(0.00);
        setPower(0.00);
        setLogMessages(prev => [...prev, { time: new Date().toLocaleTimeString(), text: "Configuration needed: Please enter a valid ESP32 IP address.", type: 'error' }]);
        return;
    }

    const fetchData = async () => {
      const url = `http://${debouncedIp}/current`;
      try {
        const response = await fetch(url);
        
        if (response.ok) {
          const data = await response.json();
          const newCurrent = parseFloat(data.current);
          
          setLiveCurrent(newCurrent);
          // Calculate power based on nominal voltage
          setPower(newCurrent * NOMINAL_VOLTAGE); 
          setIsConnected(true);
          
        } else {
          // Server returned an error (e.g., 404, 500)
          setLogMessages(prev => [...prev, { time: new Date().toLocaleTimeString(), text: `Server error: Status ${response.status} at ${url}`, type: 'error' }]);
          setIsConnected(false);
        }
      } catch (error) {
        // Network error (device offline, wrong IP, CORS issue)
        // Only log network error if previously connected to avoid spamming the log
        if (isConnected) {
             setLogMessages(prev => [...prev, { time: new Date().toLocaleTimeString(), text: `Network failure: Device unreachable at ${url}`, type: 'error' }]);
        }
        setIsConnected(false);
      }
    };

    // Fetch data every 1 second
    const interval = setInterval(fetchData, 1000); 

    // Initial fetch
    fetchData();

    return () => clearInterval(interval);
  }, [debouncedIp, isConnected]); // Re-run effect if debounced IP changes or connection status changes


  // Calculate risk-based color for current reading
  const currentRisk = liveCurrent > 4.5 ? 'text-red-500' : liveCurrent > 2.0 ? 'text-yellow-500' : 'text-green-500';

  return (
    <div className="min-h-screen bg-gray-50 dark:bg-gray-900 text-gray-800 dark:text-gray-200 p-4 sm:p-10 font-sans">
      
      {/* Header and Status */}
      <header className="flex flex-col sm:flex-row justify-between items-start sm:items-center py-6 border-b border-gray-200 dark:border-gray-700 mb-8 max-w-7xl mx-auto">
        <div className="space-y-1">
          <h1 className="text-3xl font-extrabold tracking-tight sm:text-4xl text-gray-900 dark:text-white flex items-center">
            <Bolt className="w-8 h-8 mr-3 text-indigo-500"/>
            Power Monitoring Dashboard
          </h1>
          <p className="text-lg text-gray-600 dark:text-gray-400">
            Fetching real-time data from your ESP32 web server.
          </p>
        </div>
        <div className="mt-4 sm:mt-0">
            <DeviceStatus isConnected={isConnected} ipAddress={debouncedIp} />
        </div>
      </header>
      
      {/* IP Configuration */}
      <section className="mb-8 max-w-7xl mx-auto">
          <label htmlFor="esp32-ip" className="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-2">
              ESP32 Server IP Address
          </label>
          <input
              id="esp32-ip"
              type="text"
              value={esp32Ip}
              onChange={(e) => setEsp32Ip(e.target.value)}
              placeholder="e.g., 192.168.1.100"
              className="w-full sm:w-96 p-3 border border-gray-300 rounded-lg shadow-sm focus:border-indigo-500 focus:ring-indigo-500 dark:bg-gray-700 dark:border-gray-600 dark:text-white font-mono"
          />
          <p className="mt-1 text-xs text-gray-500 dark:text-gray-400">
              Set the IP address of your ESP32 device above. Changes are applied automatically.
          </p>
      </section>

      <main className="max-w-7xl mx-auto">
        
        {/* Readings Grid */}
        <section className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6 mb-10">
          
          <CurrentCard
            title="Live Current Draw"
            value={liveCurrent}
            unit="Amperes (A)"
            icon={Zap}
            colorClass={currentRisk}
          />
          
          <CurrentCard
            title="Calculated Power"
            value={power}
            unit="Watts (W)"
            icon={Gauge}
            colorClass="text-purple-500"
          />

          <CurrentCard
            title="Nominal Voltage"
            value={NOMINAL_VOLTAGE}
            unit="Volts (V)"
            icon={Bolt}
            colorClass="text-yellow-500"
          />
        </section>
        
        {/* System Logs / Information Panel */}
        <section className="mt-10 p-6 rounded-xl bg-white dark:bg-gray-950 shadow-inner border border-gray-200 dark:border-gray-800">
          <h2 className="text-xl font-semibold mb-4 text-indigo-600 dark:text-indigo-400">
            System Log & Alerts
          </h2>
          <div className="h-48 overflow-y-auto p-3 bg-gray-50 dark:bg-gray-850 rounded-lg font-mono text-sm border border-gray-100 dark:border-gray-700">
            <p className="text-green-600 dark:text-green-400">[00:00:00]: Dashboard initialized. Waiting for data...</p>
            {logMessages.map((log, index) => (
                <p key={index} className={log.type === 'error' ? 'text-red-500 dark:text-red-400' : 'text-gray-600 dark:text-gray-400'}>
                    [{log.time}]: {log.text}
                </p>
            ))}
            {isConnected && (
                <p className="text-gray-600 dark:text-gray-400">[{new Date().toLocaleTimeString()}]: Data fetched successfully. Current: {liveCurrent.toFixed(3)} A</p>
            )}
            {liveCurrent > 4.5 && (
                 // FIX: Removed LaTeX syntax ($4.5\text{A}$)
                 <p className="text-red-600 dark:text-red-400 font-bold">[{new Date().toLocaleTimeString()}]: ALERT: Current draw exceeds HIGH threshold (4.5 A)!</p>
            )}
            {liveCurrent > 2.0 && liveCurrent <= 4.5 && (
                 // FIX: Removed LaTeX syntax ($2.0\text{A}$)
                 <p className="text-yellow-600 dark:text-yellow-400">[{new Date().toLocaleTimeString()}]: WARNING: Current draw exceeds MEDIUM threshold (2.0 A).</p>
            )}
          </div>
        </section>
      </main>

      {/* Footer */}
      <footer className="text-center mt-12 pt-6 border-t border-gray-200 dark:border-gray-700 w-full max-w-7xl mx-auto">
        <p className="text-sm text-gray-500 dark:text-gray-500">
          Built with React & Tailwind CSS. Data provided by ESP32 via HTTP polling.
        </p>
      </footer>
    </div>
  );
};

export default App;
