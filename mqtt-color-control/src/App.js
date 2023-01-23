import './App.css';
import { useEffect, useState } from 'react';
import { SketchPicker } from 'react-color';
import { Button, darken, getLuminance, Paper, Table, TableBody, TableCell, TableRow, TextField, styled, TableContainer } from '@mui/material';
const connect = window.mqtt.connect

function App() {
  const query = new Map(window.location.search?.substring(1).split('&').map(item => item.split('=')))
  
  const newLights = {}
  const [ client, setClient ] = useState(null)
  const [ server, setServer ] = useState(localStorage.getItem('server') || query.get('server'))
  const [ doConnect, setDoConnect ] = useState(false)
  const [ lights, setLights ] = useState({})
  const [ currentLight, setCurrentLight ] = useState(null)
  const [ currentColor, setCurrentColor ] = useState('#000')

  const handleServer = (ev) => {
    const serverName = ev.target.value
    setServer(serverName)
  }

  const handleMessage = (topic, message) => {
    //console.log(`RMD topic ${topic} message ${message}`)
    const parts = topic.split('/')
    newLights[parts[1]] = JSON.parse(message)
    setLights(newLights)
    //setCurrentColor(message.color)
  }

  useEffect(() => {
    if (doConnect) {
      connectServer()
    }
  }, [doConnect])

  const connectServer = (ev) => {
    localStorage.setItem('server', server)
    const newClient = connect(`ws://${server}:8080`)
    newClient.on('connect', () => {
      newClient.subscribe('/+/color')
      newClient.on('message', handleMessage)
    })
    setClient(newClient)
  }

  const handleSetColor = (light) => {
    setCurrentLight(light)
    const color = darken(lights[light].color, (1-lights[light].brightness))
    setCurrentColor(color)
  }

  const handleColorChange = ({hex}) => {
    console.log(`complete ${hex}`)
    client.publish(`/${currentLight}/color`, JSON.stringify({
      color: hex, brightness: 1
    }), {retain: true})
    console.log(`RMD currentColor ${hex}`)
    setCurrentColor(hex)
    //setCurrentLight(null)
  }

  const updateColor = ({hex}) => {
    setCurrentColor(hex)
  }

  if (!client) {
    return (
      <Paper className="App">
        <TextField label="Server" name='server' value={server} onChange={handleServer} /><br />
        <Button onClick={() => setDoConnect(true)}>Connect</Button>
      </Paper>
    )
  } else if (currentLight) {
    console.log(`RMD sketch currentColor ${currentColor}`)
    return (
      <Paper>
        <SketchPicker color={currentColor} onChange={handleColorChange} onChangeComplete={handleColorChange}/><br />
        <Button onClick={() => setCurrentLight(null)}>Set</Button>
      </Paper>
    )
  } else {
    const rows = Object.keys(lights).sort().map(light => {
      const color = darken(lights[light].color, (1-lights[light].brightness))
      const bg = getLuminance(color) > 0.5 ? 'black' : 'white'
      console.log(`RMD light ${light} calculated color ${color}`)
      const ColorButton = styled(Button)({
        backgroundColor: color,
        color: bg,
      })
      return (
        <TableRow key={light}>
          <TableCell>{light}</TableCell>
          <TableCell><ColorButton name={light} onClick={() => handleSetColor(light)} >update</ColorButton></TableCell>
        </TableRow>
      )
    })
    return (
      <TableContainer component={Paper} sx={{width: '80%', justifySelf: 'center'}}>
        <Table sx={{minWidth: 400}} size="medium">
          <TableBody>
            {rows}
          </TableBody>
        </Table>
      </TableContainer>
    );
  }
}

export default App;
