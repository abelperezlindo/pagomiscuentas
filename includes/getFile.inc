<?php
function getFile_form($form, &$form_state){

    $form['contenedor'] = array(
        '#type' => 'fieldset'
    );
    $form['contenedor'] = array(
        '#title' => t(' '),
        '#type' => 'date_popup',
        '#date_format' => 'd/m/Y',
        '#date_label_position' => 'none',
        #'#date_year_range' => '-110:+0',
        '#weight' => 0,
        // '#required' => true,
        '#description' => t('INGRESE LA FECHA DE NACIMIENTO DEL PACIENTE.'),
        '#attributes' => array(
          'placeholder' => t('FECHA DE NACIMIENTO DEL PACIENTE.'),
          'autocomplete' => 'off',
        ),
    );
    $form['contenedor']['enviar'] = array(
        '#type' => 'submit',
        '#name' => 'cargar_datos',
        '#value' => t('GENERAR ARCHIVO'),
        '#validate' => array('getFile_validate'),
        '#submit' => array('getFile_submit'),
      );
    return $form;
}
function getFile_validate($form, &$form_state){

}
function getFile_submit($form, &$form_state){

    $fileManager = new PMC();
    //$fileArray = $fileManager->file;
    $fileManager->getTxt();
    unset($fileManager);   
}

?>