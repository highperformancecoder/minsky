// import * as debug from 'debug';
// import * as express from 'express';
// import * as http from 'http';
// import * as socketIO from 'socket.io';
// const app = express();

// const logServerEvent = debug('minsky:server');
// const logUiEvent = debug('minsky:ui_event');

// const server = new http.Server(app);

// const io = socketIO(server);
// const port = process.env.PORT || 3000;

// io.on('connection', (socket) => {
//   logUiEvent('User connected');

//   socket.on('HEADER_EVENT', (message) => {
//     switch (message.target) {
//       case 'RECORD_BUTTON':
//         logUiEvent('record button clicked');
//         break;

//       case 'REFRESH_BUTTON':
//         logUiEvent('refresh button clicked');
//         // TODO: send updated canvas
//         break;

//       case 'RECORDING_REPLAY_BUTTON':
//         logUiEvent('Recording replay button clicked');
//         break;

//       case 'REVERSE_CHECKBOX_BUTTON':
//         logUiEvent('Reverse checkbox button clicked');
//         break;

//       case 'STOP_BUTTON':
//         logUiEvent('Stop button clicked');
//         break;

//       case 'STEP_BUTTON':
//         logUiEvent('Step button clicked');
//         break;

//       case 'ZOOMOUT_BUTTON':
//         logUiEvent('ZoomOut button clicked');
//         break;

//       case 'ZOOMIN_BUTTON':
//         logUiEvent('ZoomIn button clicked');
//         break;

//       case 'RESET_BUTTON':
//         logUiEvent('Reset button clicked');
//         break;

//       case 'RESETZOOM_BUTTON':
//         logUiEvent('Reset zoom button clicked');
//         break;

//       case 'ZOOMTOFIT_BUTTON':
//         logUiEvent('Zoom to fit button clicked');
//         break;

//       case 'SIMULATION_SPEED':
//         logUiEvent('Simulation speed clicked');
//         break;
//     }
//   });

//   socket.on('canvasEvent', (data) => {
//     // remove the if condition to use the mousemove events
//     if (data.type !== 'mousemove') {
//       logUiEvent('Canvas Event: ', data);
//     }
//     socket.broadcast.emit('canvasEvent', data);
//   });
// });

// io.on('new-message', (message) => {
//   io.emit(message);
// });

// export function startSocketServer() {
//   server.listen(port, () => {
//     logServerEvent(`Started on port: ${port}`);
//   });
// }
