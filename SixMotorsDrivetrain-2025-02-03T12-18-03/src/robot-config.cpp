
#include "vex.h"
using namespace vex; 
using signature = vision::signature; 
using code = vision::code;

brain  Brain; 
controller Controller1 = controller(primary);   
motor leftMotorA = motor(PORT1, ratio18_1, false);
motor leftMotorB = motor(PORT2, ratio18_1, false);
motor leftMotorC = motor(PORT3, ratio18_1, false);
motor_group LeftDriveSmart = motor_group(leftMotorA, leftMotorB,leftMotorC);


motor rightMotorA = motor(PORT12, ratio18_1, true);
motor rightMotorB = motor(PORT14, ratio18_1, true);
motor rightMotorC = motor(PORT15, ratio18_1, true);


motor_group RightDriveSmart = motor_group(rightMotorA, rightMotorB,rightMotorC);
drivetrain Drivetrain = drivetrain(LeftDriveSmart, RightDriveSmart, 319.19, 295, 40, mm, 1);

// Configuración de sensores adicionales
encoder leftEncoder = encoder(Brain.ThreeWirePort.A);  // Encoder izquierdo en puerto A
encoder rightEncoder = encoder(Brain.ThreeWirePort.B);  // Encoder derecho en puerto B
limit limitSwitch = limit(Brain.ThreeWirePort.C);       // Sensor limit en puerto C
bumper bumperSwitch = bumper(Brain.ThreeWirePort.D);    // Sensor bumper en puerto D

//-----------------------------------TREADMILL------------------------------

motor Treadmill = motor(PORT8, ratio18_1, false); // Motor principal en PORT8
motor motor_roller = motor(PORT4, ratio18_1, false); // Motor adicional en PORT4

//-----------------------------------GANCHO----------------------------------
motor gancho = motor(PORT21, ratio36_1, false); // Motor gancho en PORT21

//---------------------------------COLGARSE-----------------------------------
// A para subir, B para bajar 
motor ladderUp = motor(PORT17, ratio36_1, false);
motor ladderDown = motor(PORT18, ratio36_1,true);

//--------------------------------WALLSTAKE---------------------------------

motor wallstake = motor(PORT20, ratio36_1, true); 

void activateLadder()
{
  ladderUp.spin(forward , 100 , percent);
  ladderDown.spin(forward , 100 , percent);
}

void desactivarLadder()
{
  ladderUp.spin(reverse , 100 , percent);
  ladderDown.spin(reverse , 100 , percent);
}

// Flags para el estado de los motores
bool Treadmill_running_forward = false; // Indica si el motor está girando hacia adelante
bool Treadmill_running_reverse = false; // Indica si el motor está girando hacia atrás
bool R1_prev_state = false; // Para rastrear el estado previo del botón R1
bool R2_prev_state = false; // Para rastrear el estado previo del botón R2

void controlTreadmill() {
    // Leer el estado actual de los botones R1 y R2
    bool R1_current = Controller1.ButtonR1.pressing();
    bool R2_current = Controller1.ButtonR2.pressing();

    // Lógica para el botón R1 (hacia adelante)
    if (R1_current && !R1_prev_state) { // Detectar solo el cambio de estado (presión)
        if (Treadmill_running_forward) {
            // Si ya está corriendo hacia adelante, detener ambos motores
            Treadmill.stop();
            motor_roller.stop();
            Treadmill_running_forward = false;
        } else {
            // Si no está corriendo, iniciar hacia adelante
            Treadmill.spin(forward, 100, percent);
            motor_roller.spin(reverse, 100, percent);
            Treadmill_running_forward = true;
            Treadmill_running_reverse = false; // Asegurarse de que no esté en reversa
        }
    }

    // Lógica para el botón R2 (reversa)
    if (R2_current && !R2_prev_state) { // Detectar solo el cambio de estado (presión)
        if (Treadmill_running_reverse) {
            // Si ya está corriendo hacia atrás, detener ambos motores
            Treadmill.stop();
            motor_roller.stop();
            Treadmill_running_reverse = false;
        } else {
            // Si no está corriendo, iniciar hacia atrás
            Treadmill.spin(reverse, 100, percent);
            motor_roller.spin(forward, 100, percent);
            Treadmill_running_reverse = true;
            Treadmill_running_forward = false; // Asegurarse de que no esté hacia adelante
        }
    }

    // Actualizar los estados previos de los botones
    R1_prev_state = R1_current;
    R2_prev_state = R2_current;
}
//-----------------------------------motor_roller----------------------------------


// MOTOR DE ROLER

bool roller_running = false; 

void roller_Spin() 
{
  if (roller_running) {  
    motor_roller.stop();
  }
  else {
    motor_roller.spin(forward, 100.0,percent);
  }
  roller_running = !roller_running;
}


bool RemoteControlCodeEnabled = true;
bool DrivetrainLNeedsToBeStopped_Controller1 = true; // para izquierda
bool DrivetrainRNeedsToBeStopped_Controller1 = true;// para derecha


// codigo nuevo del gancho 
bool wasL2Pressed = false;
bool wasL1Pressed = false;

enum ControlState {
    L2_PRESSED,
    L1_PRESSED,
    NONE_PRESSED
};
ControlState getControlState() {
    if (Controller1.ButtonL2.pressing()) {
        return L2_PRESSED;
    } else if (Controller1.ButtonL1.pressing()) {
        return L1_PRESSED;
    } else {
        return NONE_PRESSED;
    }
}

void controlGancho() {
    static ControlState lastState = NONE_PRESSED;
    ControlState currentState = getControlState();
    
    switch (currentState) {
        case L2_PRESSED:
            gancho.spin(forward, 95 , rpm);
            wasL2Pressed = true;
            wasL1Pressed = false;
            break;
        
        case L1_PRESSED:
            gancho.spin(reverse, 50.0, percent);
            wasL2Pressed = false;
            wasL1Pressed = true;
            break;
        
        case NONE_PRESSED:
            if (lastState != NONE_PRESSED) {
                gancho.stop(hold);
            }
            break;
    }
    
    lastState = currentState;
}

// para el skill

void gancho_subir()
{
  gancho.spin(reverse,50.0,percent);
  wait(0.6, seconds);
  gancho.stop(hold);
}
void gancho_bajar()
{
  gancho.spin(forward,90.0,percent);
  wait(0.8, seconds);
  gancho.stop(hold);
}

// solo para autonomo sea skills o match 
void t_spin()
{
  Treadmill.spin(forward, 100.0, percent);
  motor_roller.spin(reverse, 100 , percent );

}

void t_stop()
{
  Treadmill.stop();
  motor_roller.stop();
}


void t_spin_reversa()
{
    motor_roller.spin(forward, 100.0 , percent );
    Treadmill.spin(reverse, 100.0, percent);

    
} 

// Función para detener la cinta si gira más de 90 grados
void controlar_treadmill() {
    if (leftEncoder.position(degrees) > 90) {  // Si el encoder detecta más de 90 grados de rotación
        Treadmill.stop();  // Detener la cinta
        leftEncoder.resetRotation();  // Resetear encoder a 0
    }
}

// Función para controlar el gancho con limit switch
void controlar_gancho() {
    if (limitSwitch.pressing()) {  // Si el limit switch está presionado
        gancho.stop();  // Detener el gancho
    }
}

// Función para detectar cuando el stake está en la esquina usando bumper switch
void detectar_stake_esquina() {
    if (bumperSwitch.pressing()) {  // Si el bumper switch está presionado
        Drivetrain.stop();  // Detener el drivetrain
    }
}

//================================AUTONOMO CORRESPONDIENTE AL MATCH============================
//=========== ALIANZA ROJA
void autonomous_match_red() {
    Drivetrain.setTimeout(1.0, sec);  // Establecer tiempo de espera para el drivetrain
    Treadmill.setVelocity(100.0, percent);  // Establecer la velocidad de la cinta

    // Controlar treadmill al comienzo del match
    controlar_treadmill();  // Llamar a la función para controlar la cinta

    gancho_subir();  // Subir el gancho
    Drivetrain.driveFor(reverse, 130, mm, false);  // Conducir el drivetrain en reversa por 130 mm
    wait(0.1, seconds);  // Esperar 0.1 segundos
    gancho_bajar();  // Bajar el gancho
    wait(0.5, seconds);  // Esperar 0.5 segundos
    t_spin();  // Realizar un giro t_spin
    Drivetrain.driveFor(reverse, 700, mm, false);  // Conducir el drivetrain en reversa por 700 mm
    wait(3, seconds);  // Esperar 3 segundos
    Drivetrain.turnFor(left, 50, degrees, false);  // Girar el drivetrain a la izquierda por 50 grados
    wait(1, seconds);  // Esperar 1 segundo
    Drivetrain.driveFor(forward, 320, mm, false);  // Conducir el drivetrain hacia adelante por 320 mm
    wait(2, seconds);  // Esperar 2 segundos
    Drivetrain.driveFor(reverse, 320, mm, false);  // Conducir el drivetrain en reversa por 320 mm
    wait(2, seconds);  // Esperar 2 segundos
    Drivetrain.turnFor(right, 45, degrees, false);  // Girar el drivetrain a la derecha por 45 grados
    wait(1, seconds);  // Esperar 1 segundo

    Drivetrain.setDriveVelocity(80.0, percent);  // Establecer la velocidad de conducción del drivetrain
    Drivetrain.driveFor(forward, 1500, mm, false);  // Conducir el drivetrain hacia adelante por 1500 mm
    wait(4, seconds);  // Esperar 4 segundos

    Drivetrain.driveFor(reverse, 1300, mm, false);  // Conducir el drivetrain en reversa por 1300 mm
    wait(2.5, seconds);  // Esperar 2.5 segundos
    
    Drivetrain.setDriveVelocity(50.0, percent);  // Establecer la velocidad de conducción del drivetrain
    
    Drivetrain.turnFor(left, 250, degrees, false);  // Girar el drivetrain a la izquierda por 250 grados
    wait(4, seconds);  // Esperar 4 segundos

    Drivetrain.driveFor(forward, 500, mm, false);  // Conducir el drivetrain hacia adelante por 500 mm
    wait(2, seconds);  // Esperar 2 segundos

    Drivetrain.turnFor(left, 220, degrees, false);  // Girar el drivetrain a la izquierda por 220 grados
    wait(3, seconds);  // Esperar 3 segundos

    gancho_subir();  // Subir el gancho
    wait(1, seconds);  // Esperar 1 segundo
    gancho.stop();  // Detener el gancho
    
    Drivetrain.driveFor(reverse, 1500, mm, false);  // Conducir el drivetrain en reversa por 1500 mm
    wait(3, seconds);  // Esperar 3 segundos

    Drivetrain.driveFor(forward, 900, mm, false);  // Conducir el drivetrain hacia adelante por 900 mm
    wait(2, seconds);  // Esperar 2 segundos

    // Detectar stake en la esquina
    detectar_stake_esquina();  // Llamar a la función para detectar el stake en la esquina

    Drivetrain.stop();  // Detener el drivetrain
    wait(15, msec);  // Esperar 15 milisegundos
}
//=========== ALIANZA AZUL
void autonomous_match_blue() {
    // Controlar treadmill al comienzo del match
    controlar_treadmill();  // Llamar a la función para controlar la cinta
    Drivetrain.stop();  // Detener el drivetrain
    wait(0.5, seconds);  // Esperar 0.5 segundos
}

void autonomous_skills() {
    Drivetrain.setDriveVelocity(65, percent);  // Establecer la velocidad de conducción del drivetrain
    t_spin();  // Realizar un giro t_spin
    Treadmill.setVelocity(40, percent);  // Establecer la velocidad de la cinta
    Drivetrain.driveFor(forward, 1000, mm, false);  // Conducir el drivetrain hacia adelante por 1000 mm
    wait(2, seconds);  // Esperar 2 segundos

    t_stop();  // Detener el giro t_spin
    Treadmill.setVelocity(100, percent);  // Establecer la velocidad de la cinta
    wait(1, seconds);  // Esperar 1 segundo
    Drivetrain.turnFor(left, 170, degrees, false);  // Girar el drivetrain a la izquierda por 170 grados
    wait(4, seconds);  // Esperar 4 segundos
    Drivetrain.setDriveVelocity(30, percent);  // Establecer la velocidad de conducción del drivetrain
    Drivetrain.driveFor(reverse, 500, mm, false);  // Conducir el drivetrain en reversa por 500 mm
    wait(2, seconds);  // Esperar 2 segundos

    gancho_bajar();  // Bajar el gancho
    wait(1, seconds);  // Esperar 1 segundo
    gancho.stop();  // Detener el gancho
    gancho.stop(hold);  // Mantener el gancho en su lugar

    Drivetrain.setDriveVelocity(60, percent);  // Establecer la velocidad de conducción del drivetrain
    t_spin();  // Realizar un giro t_spin
    Treadmill.setVelocity(100, percent);  // Establecer la velocidad de la cinta
    Drivetrain.turnFor(left, 138, degrees, false);  // Girar el drivetrain a la izquierda por 138 grados
    wait(4, seconds);  // Esperar 4 segundos
    Drivetrain.driveFor(forward, 600, mm, false);  // Conducir el drivetrain hacia adelante por 600 mm
    wait(3.5, seconds);  // Esperar 3.5 segundos
    Drivetrain.turnFor(right, 200, degrees, false);  // Girar el drivetrain a la derecha por 200 grados
    wait(4, seconds);  // Esperar 4 segundos

    gancho_subir();  // Subir el gancho
    wait(1, seconds);  // Esperar 1 segundo
    controlar_gancho();  // Controlar el gancho
    gancho.stop();  // Detener el gancho
    t_stop();  // Detener el giro t_spin

    // Hasta aca mete dos donas en el stake
    Drivetrain.driveFor(reverse, 500, mm, false);  // Conducir el drivetrain en reversa por 500 mm
    wait(2, seconds);  // Esperar 2 segundos

    // Aca empieza la segunda parte
    Drivetrain.driveFor(forward, 150, mm, false);  // Conducir el drivetrain hacia adelante por 150 mm
    wait(2, seconds);  // Esperar 2 segundos
    Drivetrain.turnFor(right, 80, degrees, false);  // Girar el drivetrain a la derecha por 80 grados
    wait(2, seconds);  // Esperar 2 segundos
    t_spin();  // Realizar un giro t_spin
    Treadmill.setVelocity(40, percent);  // Establecer la velocidad de la cinta
    Drivetrain.driveFor(forward, 2000, mm, false);  // Conducir el drivetrain hacia adelante por 2000 mm
    Treadmill.stop();  // Detener la cinta
    t_stop();  // Detener el giro t_spin
    wait(4, seconds);  // Esperar 4 segundos
    Drivetrain.turnFor(right, 179, degrees, false);  // Girar el drivetrain a la derecha por 179 grados
    wait(4, seconds);  // Esperar 4 segundos
    Drivetrain.setDriveVelocity(30, percent);  // Establecer la velocidad de conducción del drivetrain
    Drivetrain.driveFor(reverse, 500, mm, false);  // Conducir el drivetrain en reversa por 500 mm
    wait(3, seconds);  // Esperar 3 segundos

    gancho_bajar();  // Bajar el gancho
    wait(1, seconds);  // Esperar 1 segundo
    controlar_gancho();  // Controlar el gancho
    gancho.stop();  // Detener el gancho
    gancho.stop(hold);  // Mantener el gancho en su lugar

    t_spin();  // Realizar un giro t_spin
    Drivetrain.driveFor(forward, 500, mm, false);  // Conducir el drivetrain hacia adelante por 500 mm
    t_stop();  // Detener el giro t_spin
    Drivetrain.turnFor(right, 179, degrees, false);  // Girar el drivetrain a la derecha por 179 grados
    wait(4, seconds);  // Esperar 4 segundos
    Drivetrain.driveFor(reverse, 300, mm, false);  // Conducir el drivetrain en reversa por 300 mm
    gancho_subir();  // Subir el gancho
    wait(1, seconds);  // Esperar 1 segundo
    controlar_gancho();  // Controlar el gancho
    gancho.stop();  // Detener el gancho
    Drivetrain.driveFor(reverse, 300, mm, false);  // Conducir el drivetrain en reversa por 300 mm

    // Detectar stake en la esquina
    detectar_stake_esquina();  // Llamar a la función para detectar el stake en la esquina
}
int rc_auto_loop_function_Controller1() {
  // process the controller input every 20 milliseconds
  // update the motors based on the input values

  // programcion


 
  //--------------------------------------------------------------------- posible programacion para autonomo skills


  // gancho_subir();
  // // Treadmill.spin(forward,150,rpm);
  // // motor_roller.spin(reverse,150,rpm);
  // t_spin();
  
  // Drivetrain.drive(reverse,100,rpm);
  // wait(1.25,seconds);
  
  // Drivetrain.stop();

  // gancho_bajar();
  // wait(0.4,seconds);


  // Drivetrain.drive(reverse,100,rpm);
  // wait(0.8,seconds);

  // //girar para ir hacia la esquina

  // Drivetrain.turn(right,100,rpm);
  // wait(0.2, seconds);

  // // agarrar la dona
  // Drivetrain.drive(forward,100,rpm);
  // wait(1.7,seconds);
  // // girar sobre eje para retrocede
  // Drivetrain.turn(right,100,rpm);
  // wait(2.2, seconds);
  // Drivetrain.stop();
  // t_stop();

  // // suelta stake 
  // gancho_subir();
  // wait(0.5,seconds);

  // // retroceder para ir a la esquina
  // Drivetrain.drive(reverse,100,rpm);
  // wait(0.65,seconds);

  // // avanza
  // Drivetrain.drive(forward,100,rpm);
  // wait(0.5,seconds);

  // Drivetrain.turn(left,100,rpm);
  // wait(0.46, seconds);

  // // avanza para agarrar la dona
  // Drivetrain.drive(forward,100,rpm);
  // wait(1.9,seconds);

  // // girar para agarrar el segundo stake
  // Drivetrain.turn(left,100,rpm);
  // wait(1.34, seconds);

  // //retroceder
  // Drivetrain.drive(reverse,100,rpm);
  // wait(2,seconds);

  // Drivetrain.stop();

  // gancho_bajar();
  // wait(0.5,seconds);

  // // girar para ir a la otra esquina
  // Drivetrain.turn(left,100,rpm);
  // wait(0.9, seconds);

  // Drivetrain.stop();

  // gancho_subir();
  // wait(0.5,seconds);

  // // mete en la esquina
  // Drivetrain.drive(reverse,100,rpm);
  // wait(2.1,seconds);

  // // contrinua por el tercer stake

  // Drivetrain.drive(forward,100,rpm);
  // wait(2,seconds);
  

  // // para que gire 90
  // LeftDriveSmart.spinFor(reverse,1,seconds);
  // wait(0.9, seconds);



  // Drivetrain.drive(reverse,100,rpm);
  // wait(4,seconds);

// // hasta aca anda



// la primera parte no funciona debido a que nos equivocamos de lado 
// segunda parte del autonomo 
//--------------------------------------------------------------------------------------------------------------------------------------------------
// // iniciamos subiendo y  ir a agarrar el stake 
// Drivetrain.setTimeout(1,seconds);
// gancho_subir();// sube el gancho al inicio
// Drivetrain.drive(reverse, 100,rpm);
// wait(0.5,seconds);
// Drivetrain.turnFor(65,degrees,75);
// wait(1,seconds);
// Drivetrain.drive(reverse,100,rpm);
// wait(1,seconds);

// Drivetrain.stop();

// // ahora agarra el primer stake
// gancho_bajar();
// wait(0.5,seconds);

// // proceso de agarrar las donas

// t_spin(); // comienza a funcionar los rollers

// Drivetrain.turn(left,100,rpm);  // gira a la izquierda para agarrar la primera dona
// wait(0.3,seconds);

// Drivetrain.drive(forward,100,rpm);  // agarra la dona
// wait(1,seconds);

// Drivetrain.stop(); 
// wait(0.7,seconds);

// // proceso para agarra la segunda dona 
// Drivetrain.drive(reverse,200,rpm);
// wait(0.8,seconds);

// Drivetrain.stop();

// Drivetrain.turn(right,100,rpm);// gira para agarrar la segunda dona
// wait(0.77,seconds);

// Drivetrain.stop();

// // avanza para agarrar la segunda dona
// Drivetrain.drive(forward,100,rpm);
// wait(1.4,seconds);

// // proceso para agarrar la tercera dona
// Drivetrain.stop();

// Drivetrain.turn(left,100,rpm); // gira para agarrar la tercera dona
// wait(0.28,seconds);

// Drivetrain.stop();
// wait(0.5,seconds);

// Drivetrain.drive(forward,100,rpm);
// wait(1.4,seconds);// agarra la tercera dona

// Drivetrain.stop();
// wait(0.5,seconds);

// // gira para encestar en la esquina positiva
// Drivetrain.turn(right,100,rpm);
// wait(0.5,seconds);

// Drivetrain.stop();
// wait(0.5,seconds);

// //subir el gancho 
// gancho_subir();
// wait(0.5,seconds);


// //retrocede para meter en la esquina positiva
// Drivetrain.drive(reverse,200,rpm);
// wait(1.4,seconds);



// // hasta aca anda super bien

// t_stop();

// // Continuacion del skill

// Drivetrain.stop();
// wait(0.5,seconds);

// // gira 90 grados para ir a agarrar la cuarta dona
// Drivetrain.turn(right,100,rpm);
// wait(1,seconds);

// // avanza para precargar la tercera dona 
// Drivetrain.drive(forward,200,rpm);
// wait(0.8,seconds);


// Drivetrain.stop();
// wait(0.5,seconds);

// t_spin();

// Drivetrain.drive(forward,200,rpm);
// wait(0.5,seconds);

// t_stop();

while(true) {
if(RemoteControlCodeEnabled) 
    {

      int drive_Left_Speed = Controller1.Axis3.position();
      int drive_Right_Speed = Controller1.Axis2.position();
//
      if (drive_Left_Speed < 5 && drive_Left_Speed > -5) {
        if (DrivetrainLNeedsToBeStopped_Controller1) {
          LeftDriveSmart.stop();
          DrivetrainLNeedsToBeStopped_Controller1 = false;
        }
      } else {
        DrivetrainLNeedsToBeStopped_Controller1 = true;
      }
  //
      if (drive_Right_Speed < 5 && drive_Right_Speed > -5) {
        
        if (DrivetrainRNeedsToBeStopped_Controller1) {
          RightDriveSmart.stop();
          DrivetrainRNeedsToBeStopped_Controller1 = false;
        }
      } else {
        DrivetrainRNeedsToBeStopped_Controller1 = true;
      }
      
      if (DrivetrainLNeedsToBeStopped_Controller1) {
        LeftDriveSmart.setVelocity(drive_Left_Speed, percent);
        LeftDriveSmart.spin(forward);
      }
      if (DrivetrainRNeedsToBeStopped_Controller1) {
        RightDriveSmart.setVelocity(drive_Right_Speed, percent);
        RightDriveSmart.spin(forward);
      }
      

      //Controller1.ButtonR2.pressed(Treadmill_Spin_Reverse);
     // Controller1.ButtonR1.pressed(Treadmill_Spin);


      




//============Funcion que se llama para verificar el funcionamiento del treadmill      
      controlTreadmill();

      Controller1.ButtonUp.pressed(autonomous_match_red);
      Controller1.ButtonDown.pressed(autonomous_match_blue);
      Controller1.ButtonLeft.pressed(autonomous_skills);

      if(Controller1.ButtonX.pressing()){
        wallstake.spin(forward, 100, percent); 
      }
      else if(Controller1.ButtonY.pressing()){
        wallstake.spin(reverse, 100, percent); 
      }
      else
      {
        wallstake.stop();
      }

      if(Controller1.ButtonA.pressing())
      {
        activateLadder();
      }
      else if (Controller1.ButtonB.pressing()) {
        desactivarLadder();
      }
      else {
        ladderUp.stop();
        ladderDown.stop(hold);
      }

      // parte del gancho con clases y funciones 

      controlGancho();

    }
    // wait before repeating the process
    wait(20, msec);
  }
  return 0;
}



/**
 * Used to initialize code/tasks/devices added using tools in VEXcode Pro.
 * 
 * This should be called at the start of your int main function.
 */
void vexcodeInit( void ) {
  task rc_auto_loop_task_Controller1(rc_auto_loop_function_Controller1);
}