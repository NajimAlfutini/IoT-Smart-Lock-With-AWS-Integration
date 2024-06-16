import React, { useState } from 'react';
import moment from 'moment';


const SearchBar = ({ logs, setFilteredLogs }) => {
    const [searchName, setSearchName] = useState('');
    const [startTime, setStartTime] = useState('');
    const [endTime, setEndTime] = useState('');

    const handleSearch = () => {
        let filteredLogs = logs.filter(log => {
            let logTime = moment(log.timestamp, "DD/MM/YYYY HH:mm:ss");
            let start = startTime ? moment(startTime) : null;
            let end = endTime ? moment(endTime) : null;
            return log.Name.toLowerCase().includes(searchName.toLowerCase()) && 
                   (!start || logTime.isSameOrAfter(start)) && 
                   (!end || logTime.isSameOrBefore(end));
        });
        setFilteredLogs(filteredLogs);
    };
    
    
    return (
        <div>
            <input type="text" className='SearchName' placeholder="Search by name" value={searchName} onChange={(e) => setSearchName(e.target.value)} />
            <input type="datetime-local" value={startTime} onChange={(e) => setStartTime(e.target.value)} />
            <input type="datetime-local" value={endTime} onChange={(e) => setEndTime(e.target.value)} />
            <button onClick={handleSearch}>Search</button>
        </div>
    );
};

export default SearchBar;
