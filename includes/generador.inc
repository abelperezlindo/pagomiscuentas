<?php 
class PMC {
	public $file = array();
	function __construct(){
		// nombre del archivo COBXXXX.DDMMAA  (XXX es número de empresa, DDMMAA es fecha de acreditación de fondos a la empresa)
		$data = $this->getData();
		if(!$data){
			watchdog('pmc', t('Se podujo un error al intentar obtener los datos necesarios para crear el archivo de facturas.'));
			drupal_set_message(t('Se podujo un error alintentar generar el archivo.'),'error');
		} 
		/* Nombre de archivo*/
		$this->file['filename'] = 'FAC'.$data['nro_empresa'].'.'.$data['fecha_acred_name'];
		/*
		* Header
		*/
		$header = '0400'; //id de registro y codigo prisma
		$header .= $data['nro_empresa']; //4digitos
		$header .= $data['fecha_acred']; //AAAAMMDD
		$header = str_pad($header,280,'0',STR_PAD_RIGHT); //Rellenar con ceros hasta 280 char
		$header .= "\n"; //final de la linea
		$this->file['header'] = $header;
		/*
		* Trailer
		*/
		$trailer = '9400'; //id de registro y codigo prisma
		$trailer .= $data['nro_empresa']; //4 digitos
		$trailer .= $data['fecha_acred']; //AAAAMMDD
		$trailer .= $data['countFormat']; //cantidad de registros con formato
		$trailer = str_pad($trailer,30,'0',STR_PAD_RIGHT); //Rellenar con ceros hasta pos 30, 7 ceros (Filer)
		$trailer .= $data['montoTotalFormat']; //monto total formateado
		$trailer = str_pad($trailer,280,'0',STR_PAD_RIGHT); //Rellenar con ceros (Filer)
		$this->file['trailer'] = $trailer;
		/*
		* Detalle
		*/
		$this->file['detalle'] = array();
		
		for($i = 0; $i < count($data['detalle']); $i++){
			$detalle  = '5'; //id de registro
			$detalle .= $data['detalle'][$i]['referencia']; //nro ref cliente 
			$detalle .= $data['detalle'][$i]['factura']; //nro ref factura
			$detalle .= '0'; // moneda peso
			$detalle .= $data['detalle'][$i]['vencimiento']; //primer venc
			$detalle .= $data['detalle'][$i]['importe']; //monto primer venc		
			$detalle .= $data['detalle'][$i]['vencimientoDos']; //segundo venc
			$detalle .= $data['detalle'][$i]['importe']; //monto segundo venc
			$detalle .= $data['detalle'][$i]['vencimientoTres']; //tercer venc
			$detalle .= $data['detalle'][$i]['importe']; //monto tercer venc	
			// A los vencimientos lo maneja pmc o talleres?
			#$detalle = str_pad($detalle,98,'0',STR_PAD_RIGHT); //Rellenamos con cero la parte de los vencimientos
			$detalle = str_pad($detalle,117,'0',STR_PAD_RIGHT); //Rellenar con ceros hasta 117 char, 19 ceros
			$detalle .= $data['detalle'][$i]['referencia']; // se repite ref socio (sirve para informar que el id de socio cambio)
			$detalle .= 'FACT CUOTA PERIODO '.$data['detalle'][$i]['periodo'].' CLUB TALLERES'; //mensaje tiket 40 chars
			$detalle .= 'CUOTA '.$data['detalle'][$i]['periodo'].' '; // mensaje de pantalla
			$detalle = str_pad($detalle,251,' ',STR_PAD_RIGHT);
			$detalle = str_pad($detalle,280,'0',STR_PAD_RIGHT); //codigo de barra (60 chars) mas filer hasta 280 chars
			$detalle .= "\n";
			$this->file['detalle'][] = $detalle;
			
		}
	}
	private function getData(){
		//Obtenemos fecha actual desde la base de datos
		$fecha = db_query('SELECT date_format(CURRENT_DATE(), \'%Y%m%d\') AS AAAAMMDD, date_format(CURRENT_DATE(), \'%d%m%y\') AS DDMMAA;')->fetchObject();
		//Buscamos las cuotas a pagár
		$path = drupal_get_path('module', 'pmc'); //ruta del modulo
		$query = file_get_contents($path."/sql/getDeudaSocios.sql"); //encadenar archivo a variable
		if(!$query){
			//Guardamos en el log, pero seguimos con la ejecución.
			drupal_set_message('Consulta no encontrada', 'error'); 
		}
		$resultado = array();
		$resultado['nro_empresa'] = '0470';
		$resultado['fecha_acred'] = $fecha->AAAAMMDD;
		$resultado['fecha_acred_name'] = $fecha->DDMMAA;
		$resultado['count'] = 0; //cantidad de cuotas a pagar
		$resultado['detalle'] = array();
		$resultado['montoTotal'] = 0;
		//Si no hay cuotas a pagar lo retornamos sin datos en Detalle
		$result = db_query($query)->fetchAll();
		if(empty($result[0])) return $resultado;
		//iteramos cargando los registros del detalle
		foreach($result as $record){
			$row = (array)$record;
			$resultado['detalle'][$resultado['count']] = array(
				'referencia' => $row['idSocio'], 
				'factura' => $row['idFactura'], 
				'vencimiento' => $row['vencimiento'],
				'vencimientoDos' => $row['vencimientoDos'],
				'vencimientoTres' => $row['vencimientoTres'],
				'importe' => $row['montoFormat'],
				'periodo' => $row['periodo']
			);
			$resultado['count'] ++;
			$resultado['montoTotal'] += $row['monto'];
		}
		//formateamos el numero total  de registros como una cadena de 7 chars con ceros a la izq
		$resultado['countFormat'] = str_pad(strval($resultado['count']),7,'0',STR_PAD_LEFT);
		$resultado['montoTotalFormat'] = $aux_total;
		//formateamos el Monto total como una cadena de 7 chars con ceros a la izq
		$aux_total = str_replace('.','',strval($resultado['montoTotal']));
		str_pad($aux_total,7,'0',STR_PAD_LEFT);
		$resultado['montoTotalFormat'] = $aux_total;
		return $resultado;
	}

	public function getTxt(){ 
		drupal_add_http_header('Content-Type', 'text; charset=utf-8');
		drupal_add_http_header('Content-Disposition', 'attachment; filename="'.$this->file['filename'].'"');
		ob_end_clean (); //borramos el buffer
		$fp = fopen('php://output', 'w');
		fseek($fp,0); //puntero al inicio
		fwrite($fp, $this->file['header']); //escribimos el header
		foreach($this->file['detalle'] as $detalle){
			fwrite($fp, $detalle); //escribimos los registros de factura
		}
		fwrite($fp, $this->file['trailer']); //escribimos el trailer
		fclose($fp); 
		drupal_exit();
	}

}