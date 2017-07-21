; Copyright 2017, Jeffrey E. Bedard
(load "libjb/libconvert.scm")
(define convert-keys
 (lambda (in_filename out_filename)
  (define parse-keys ; convert each line to a cons cell
   (lambda (i o) (and-let* ((line (read-line i)) ((not (eof-object? line)))
			    ((> (string-length line) 1)))
		  (let* ((key (string-car line)) (value (string-cdr line))
			 (next (parse-keys i o)) (cell (cons key value)))
		   (if (eq? #f next) (cons cell '()) (cons cell next))))))
  (let ((i (open-input-file in_filename))
	(o (open-output-file out_filename))
	(guard "JBWM_JBWMKEYS_H")
	(format-cell ; function to pass to map
	 (lambda (datum out_port)
	  (display (string-append "\tJBWM_KEY_" (car datum)
		    " = XK_" (cdr datum) ",\n") out_port)))
	(compare-cell (lambda (a b) (string<? (car a) (car b)))))
   (define keys-data (sort (parse-keys i o) compare-cell))
   (begin-include guard o)
   (c-add-include "<X11/keysym.h>" o)
   (begin-enum-definition "JBWMKeys" o)
   (map (lambda (n) (format-cell n o)) keys-data)
   (display keys-data)
   (display "};\n" o)
   (c-add-include "\"key_combos.h\"" o)
   (end-include guard o)
   (close-port i)
   (close-port o))))

	(convert-keys "keys.txt" "JBWMKeys.h")
