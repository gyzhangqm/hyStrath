
Info << "U_pos: " << U_pos << endl;
Info << "U_neg: " << U_neg << endl;

Info << "mesh.Sf(): " << mesh.Sf() << endl;
Info << "mesh.magSf(): " << mesh.magSf() << endl;

surfaceScalarField phiv_pos("phiv_pos", (U_pos & mesh.Sf())); //- Step II, + direction
surfaceScalarField phiv_neg("phiv_neg", (U_neg & mesh.Sf())); //- Step II, - direction

Info << "phiv_pos: " << phiv_pos << endl;
Info << "phiv_neg: " << phiv_neg << endl;

surfaceScalarField cSf_pos
(
    "cSf_pos",
    fvc::interpolate(c, pos, "reconstruct(T)")*mesh.magSf()*sign(mesh.Sf().component(0))
);
surfaceScalarField cSf_neg
(
    "cSf_neg",
    fvc::interpolate(c, neg, "reconstruct(T)")*mesh.magSf()*sign(mesh.Sf().component(0))
);

Info << "cSf_pos: " << cSf_pos << endl;
Info << "cSf_neg: " << cSf_neg << endl;

surfaceScalarField cf //- Step I, cf definition no 1
(
    "cf",
    0.5*(cSf_pos + cSf_neg)
);

surfaceScalarField Mach_pos //- Eq.69
(
    "Mach_pos",
    phiv_pos/cf
);

surfaceScalarField Mach_neg
(
    "Mach_neg",
    phiv_neg/cf
);

Info << "Mach_pos: " << Mach_pos << endl;
Info << "Mach_neg: " << Mach_neg << endl;

surfaceScalarField meanSqrMach //- Step III
(
    "meanSqrMach",
    (sqr(phiv_pos) + sqr(phiv_neg))/(2.0*sqr(cf))
);

Info << "meanSqrMach: " << meanSqrMach << endl;

surfaceScalarField meanSqrMachZero //- Step IV
(
    "meanSqrMachZero",
    min(1.0, max(max(meanSqrMach, sqrMachInf), 1e-6)) //- 1e-6: safe-guard for fluid initially at rest
);

Info << "meanSqrMachZero: " << meanSqrMachZero << endl;

surfaceScalarField scalingFunction //- Step V
(
    "fa",
    sqrt(meanSqrMachZero)*(2.0-sqrt(meanSqrMachZero))
);

Info << "scalingFunction: " << scalingFunction << endl;

scalar AUSM_Kp = 0.25;
scalar AUSM_Ku = 0.75;
scalar AUSM_sigma = 1.0;
scalar AUSM_beta = 1.0/8.0;

surfaceScalarField AUSM_alpha
(
    "AUSM_alpha",
    3.0/16.0*(-4.0+5.0*sqr(scalingFunction))
);

Info << "AUSM_alpha: " << AUSM_alpha << endl;

Info << "splitMachDeg1_pos(Mach_pos, AUSM_beta): " << splitMachDeg1_pos(Mach_pos) << endl;
Info << "splitMachDeg1_pos(Mach_neg, AUSM_beta): " << splitMachDeg1_pos(Mach_neg) << endl;

Info << "splitMachDeg1_neg(Mach_pos, AUSM_beta): " << splitMachDeg1_neg(Mach_pos) << endl;
Info << "splitMachDeg1_neg(Mach_neg, AUSM_beta): " << splitMachDeg1_neg(Mach_neg) << endl;

Info << "splitMachDeg2_pos(Mach_pos, AUSM_beta): " << splitMachDeg2_pos(Mach_pos) << endl;
Info << "splitMachDeg2_pos(Mach_neg, AUSM_beta): " << splitMachDeg2_pos(Mach_neg) << endl;

Info << "splitMachDeg2_neg(Mach_pos, AUSM_beta): " << splitMachDeg2_neg(Mach_pos) << endl;
Info << "splitMachDeg2_neg(Mach_neg, AUSM_beta): " << splitMachDeg2_neg(Mach_neg) << endl;

Info << "splitMachDeg4_pos(Mach_pos, AUSM_beta): " << splitMachDeg4_pos(Mach_pos, AUSM_beta) << endl;

Info << "splitMachDeg4_neg(Mach_neg, AUSM_beta): " << splitMachDeg4_neg(Mach_neg, AUSM_beta) << endl;

surfaceScalarField Machf //- Step VI
(
    "Machf",
    splitMachDeg4_pos(Mach_pos, AUSM_beta) + splitMachDeg4_neg(Mach_neg, AUSM_beta)
);

Machf -= AUSM_Kp/scalingFunction*max(1.0-AUSM_sigma*meanSqrMach, 0.0)*(p_neg-p_pos)
  / (0.5*(rho_pos+rho_neg)*sqr(cf/mesh.magSf()));
  
Info << "Machf: " << Machf << endl;

Info << "p_pos: " << p_pos << endl;

Info << "p_neg: " << p_neg << endl;

Info << "splitPressureDeg5_pos(Mach_pos, AUSM_alpha): " << splitPressureDeg5_pos(Mach_pos, AUSM_alpha) << endl;

Info << "splitPressureDeg5_neg(Mach_neg, AUSM_alpha): " << splitPressureDeg5_neg(Mach_neg, AUSM_alpha) << endl;

surfaceScalarField pf 
(
    "pf",
    splitPressureDeg5_pos(Mach_pos, AUSM_alpha)*p_pos + splitPressureDeg5_neg(Mach_neg, AUSM_alpha)*p_neg
);

Info << "pf without Ku: " << pf << endl;

pf -= AUSM_Ku*(rho_pos+rho_neg)*(phiv_neg-phiv_pos)*(scalingFunction*cf)/sqr(mesh.magSf())
   *splitPressureDeg5_pos(Mach_pos, AUSM_alpha)*splitPressureDeg5_neg(Mach_neg, AUSM_alpha);

Info << "pf: " << pf << endl;

// Use amaxSf for the maximum positive and negative fluxes
// estimated by the AUSM+-up scheme
surfaceScalarField amaxSf = max(mag(phiv_pos), mag(phiv_neg));

Info << "amaxSf:" << amaxSf << endl;

